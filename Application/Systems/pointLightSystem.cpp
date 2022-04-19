#include "pointLightSystem.hpp"
#include "controller.hpp"
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

    //the point light push constant data, something like a uniform in opengl, must be aligned!!!!
    struct PointLightPushConstants {
        glm::vec4 position{};//must be aligned by multiple of 16 bytes that's why it's vec4 
        glm::vec4 color{};//must be aligned by multiple of 16 bytes that's why it's vec4, can use alignas(16) glm::vec3 color
        float radius;
    };


    PointLightSystem::PointLightSystem(
        NKDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
        : m_Device{ device } {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    PointLightSystem::~PointLightSystem() {
        vkDestroyPipelineLayout(m_Device.device(), pipelineLayout, nullptr);
    }

    void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;//which stages will have access to this push constant range 
        pushConstantRange.offset = 0;//must be a multiple of 4
        pushConstantRange.size = sizeof(PointLightPushConstants);//must be a multiple of 4

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(m_Device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void PointLightSystem::update(FrameInfo& frameInfo, GlobalUbo& ubo) {

        auto rotateLight = glm::rotate(glm::mat4(1.f), 0.5f * frameInfo.frameTime, { 0.f, -1.f, 0.f });
        int lightIndex = 0;
        for (auto& kv : frameInfo.gameObjects) {

            auto& obj = kv.second;
            if (obj.pointLight == nullptr) continue;

            assert(lightIndex < MAX_LIGHTS && "Point lights exceed maximum specified");

            // update light position
            obj.transform.translation = glm::vec3(rotateLight * glm::vec4(obj.transform.translation, 1.f));
            //obj.transform.translation = glm::vec4(obj.transform.translation, 1.f);

            // copy light to ubo
            //update the first one to be in the camera position 
            if (lightIndex == 0) {
                //if following camera update with the camera position 
                if (mFollowCamera) {
                    ubo.pointLights[lightIndex].position = ubo.cameraEyePos;
                    obj.transform.translation = ubo.cameraEyePos;
                }
                else {
                    ubo.pointLights[lightIndex].position = mStaticCameraPos;
                    obj.transform.translation = mStaticCameraPos;
                }
            }
            else {
                ubo.pointLights[lightIndex].position = glm::vec4(obj.transform.translation, 1.f);
            }
            ubo.pointLights[lightIndex].color = glm::vec4(obj.color, obj.pointLight->lightIntensity);

            lightIndex += 1;
        }
        ubo.numLights = lightIndex;
    }

    void PointLightSystem::inputUpdate(NkGameObject& viewerObject) {
        //check light following camera
        if (KeyManager.isKeyTriggered(VK_SPACE)) {
            mFollowCamera = !mFollowCamera;//toggle between states
            if (!mFollowCamera) {
                mStaticCameraPos = { viewerObject.transform.translation,1.f };
            }
        }
    }

    void PointLightSystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        NKPipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.bindingDescriptions.clear();
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        m_Pipeline = std::make_unique<NKPipeline>(
            m_Device,
            "Shaders/point_light.vert.spv",
            "Shaders/point_light.frag.spv",
            pipelineConfig);
    }

    void PointLightSystem::render(FrameInfo& frameInfo) {

        m_Pipeline->bind(frameInfo.commandBuffer);

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
            if (obj.pointLight == nullptr) continue;

            PointLightPushConstants push{};
            push.position = glm::vec4(obj.transform.translation, 1.f);
            push.color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
            push.radius = obj.transform.scale.x;

            vkCmdPushConstants(
                frameInfo.commandBuffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(PointLightPushConstants),
                &push);
            vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
        }
    }

}  // namespace lve