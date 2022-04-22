#include "rendererSystem.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <stdexcept>

namespace nekographics {

    /*
    Pros & Cons of Push Constant Data

    Pros
    -quick to implement 
    -high perforamcne for frequently updating data

    Cons
    -limited in size ( only 128 bytes guaranteed)
    -Tied to draw calls making aggregation of draw calls difficult/impossible 
    */
    //the simple push constant data 
    struct SimplePushConstantData {
        glm::mat4 modelMatrix{ 1.f };//the model matrix 
        glm::mat4 normalMatrix{ 1.f };
    };

    SimpleRenderSystem::SimpleRenderSystem(NKDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
        : m_systemDevice{ device } {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    SimpleRenderSystem::~SimpleRenderSystem() {
        vkDestroyPipelineLayout(m_systemDevice.device(), pipelineLayout, nullptr);
    }

    void SimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
        //setting the push constant range 
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;//which stages will have access to this push constant range 
        pushConstantRange.offset = 0;//must be a multiple of 4, offset is mainly for if you are using seperate ranges for the vertex and fragment shaders
        pushConstantRange.size = sizeof(SimplePushConstantData);//must be a multiple of 4

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

        //setting the pipeline layout to 
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());//the number of layouts
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();//pass data other than a vertex data to our vertex & fragment shader, this includes textures & UBO
        pipelineLayoutInfo.pushConstantRangeCount = 1;//there is a push constant range 
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;//sending a small amount of data to our shader programs efficiently 

        //creating pipeline layout 
        if (vkCreatePipelineLayout(m_systemDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
        //using this pipeline 
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        NKPipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;//setting the render pass, render pass describes the structure & format of our framebuffer object & their attachments
        pipelineConfig.pipelineLayout = pipelineLayout;

        //making the pipeline base off the shader file for the skull 
        m_systemPipeline = std::make_unique<NKPipeline>(
            m_systemDevice,
            "Shaders/shaderSkull.vert.spv",
            "Shaders/shaderSkull.frag.spv",
            pipelineConfig);

        //making the pipeline base off the shader file for the car
        m_systemPipelineCar = std::make_unique<NKPipeline>(
            m_systemDevice,
            "Shaders/shaderCar.vert.spv",
            "Shaders/shaderCar.frag.spv",
            pipelineConfig);
    }

    void SimpleRenderSystem::renderGameObjects(
        FrameInfo& frameInfo) {

        ////check the frame info for the type of object you are rendering 
        //m_systemPipeline->bind(frameInfo.commandBuffer);//binding the pipeline

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0,
            1,
            &frameInfo.globalDescriptorSet,
            0,
            nullptr);

        for (auto& kv : frameInfo.gameObjects) {
            auto& obj = kv.second;
            if (obj.model == nullptr) continue;
            
            //check which model it is to bind the respective shader 
            if (kv.first == 0) {
                m_systemPipeline->bind(frameInfo.commandBuffer);//binding the pipeline
            }
            else if (kv.first == 1) {
                m_systemPipelineCar->bind(frameInfo.commandBuffer);//binding the pipeline
            }

            SimplePushConstantData push{};//creating a simple constant data 
            //initialize the push constant data 
            push.modelMatrix = obj.transform.mat4();
            push.normalMatrix = obj.transform.normalMatrix();

            vkCmdPushConstants(
                frameInfo.commandBuffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(SimplePushConstantData),
                &push);

            //check if there are child models 
            if (obj.model->getHasChildModels()) {
                //loop through all the child models 
                for (auto& childModels : obj.model->getChildModels()) {
                    childModels->bind(frameInfo.commandBuffer);//binding the pipeline 
                    childModels->draw(frameInfo.commandBuffer);//draw all the child models 
                }
            }
            else {
                obj.model->bind(frameInfo.commandBuffer);//binding the pipeline 
                obj.model->draw(frameInfo.commandBuffer);//drawing 
            }
        }
    }

} 
