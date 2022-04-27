/******************************************************************************/
/*!
\file   3DCube.cpp
\author Wilfred Ng Jun Hwee
\par    DP email: junhweewilfred.ng[at]digipen.edu
\par    course: csd2150
\par    Final Mesh Viewer
\date   04/15/2022
\brief
	This is the implementation file of the application
*/
/******************************************************************************/

//includes
#include "3DMeshViewer.hpp"
#include "controller.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

//std
#include <chrono>
#include <iostream>

#define MAX_NUM_LIGHTS 4
#define MIN_NUM_LIGHTS 1

namespace nekographics {

	void gameApp::pipelineLayout() {
		/**************
		Creating Uniform Buffers
		**************/
		uboBuffers.resize(nekographics::NKSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++) {
			uboBuffers[i] = std::make_unique<nekographics::NKBuffer>(
				m_vkDevice,
				sizeof(nekographics::GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			uboBuffers[i]->map();
		}

		/**************
		Creating Descriptor Sets Layout
		**************/
		NKDescriptorSetLayout::Builder tmpBuilder = NKDescriptorSetLayout::Builder(m_vkDevice);//temporary builder 
		tmpBuilder.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS);//for the UBO 

		//adding in the textures to the descriptor sets 
		for (int i = 1; i < m_vktexture.textureImageVec.size() + 1; ++i) {
			tmpBuilder.addBinding(i, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS);
		}
		globalSetLayout = tmpBuilder.build();//building the set layout 

		/**************
		Creating Descriptor Sets
		**************/
		globalDescriptorSets.resize(nekographics::NKSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++) {

			std::vector<VkDescriptorImageInfo> imageInfoVec;
			for (int j = 0; j < m_vktexture.textureImageViewVec.size(); ++j) {
				//setting the image info 
				VkDescriptorImageInfo imageInfo{};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = m_vktexture.textureImageViewVec[j];
				imageInfo.sampler = m_vktexture.textureSamplerVec[j];
				imageInfoVec.emplace_back(imageInfo);
			}

			//setting the buffer info 
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			nekographics::NkDescriptorWriter tmpWriter = nekographics::NkDescriptorWriter(*globalSetLayout, *globalPool);//creating the temporary writer 
			tmpWriter.writeBuffer(0, &bufferInfo);//writing buffer into the writer 
			//adding write image into the writer 
			for (int k = 1, m = 0; k < m_vktexture.textureImageVec.size() + 1; ++k, ++m) {
				tmpWriter.writeImage(k, &imageInfoVec[m]);
			}
			tmpWriter.build(globalDescriptorSets[i]);//building the descriptor sets 
		}
	}

	gameApp::gameApp() {
		/**************
		Creating Descriptor Pool
		**************/
		globalPool =
			NKDescriptorPool::Builder(m_vkDevice)
			.setMaxSets(NKSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, NKSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, NKSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, NKSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, NKSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, NKSwapChain::MAX_FRAMES_IN_FLIGHT)			
			
			//for the vintage textures 
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, NKSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, NKSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, NKSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, NKSwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();

	}

	/***********
	Destructor for application 
	************/
	gameApp::~gameApp() {}

	/***********
	loading the dds textures 
	************/
	void gameApp::loadTextures(const std::string& textures) {
		m_vktexture.createTextureImageDDSMIPMAPS(textures);//creating the texture image 
	}

	/***********
	Creating and setting the init transform of point lights 
	************/
	void gameApp::loadPointLights(const int& numberOfLights) {

		/**************
		Creating point lights 
		**************/
		std::vector<glm::vec3> lightColors{
			{1.f, 1.f, 1.f},
			{1.f, .1f, .1f},
			{0.f, 1.f, .1f},
			{0.f, .1f, 1.1f},
		};

		//sanity check for the number of lights 
		if (numberOfLights > MAX_NUM_LIGHTS) {
			lightColors.resize(MAX_NUM_LIGHTS);//set it to the max 
		}else {
			lightColors.resize(numberOfLights);
		}

		if (numberOfLights <= 0) {
			lightColors.resize(MIN_NUM_LIGHTS);
		}

		/**************
		Setting the lights initial translation 
		**************/
		for (int i = 0; i < lightColors.size(); i++) {
			auto pointLight = NkGameObject::makePointLight(1.f);
			pointLight.color = lightColors[i];
			auto rotateLight = glm::rotate(
				glm::mat4(1.f),
				(i * glm::two_pi<float>()) / lightColors.size(),
				{ 0.f, -1.f, 0.f });
			pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -2.f, -1.f, 1.f));
			gameObjects.emplace(pointLight.getId(), std::move(pointLight));
		}
	}

	/***********
	Application draw call 
	************/
	void gameApp::draw(NKCamera& camera, SimpleRenderSystem& renderer , PointLightSystem& pointLightRenderer,FrameInfo& frameInfo, VkCommandBuffer& commandBuffer) {
		UNREFERENCED_PARAMETER(camera);

		//check for begin frame 
		m_vkRenderer.beginSwapChainRenderPass(commandBuffer);//begin renderpass
		renderer.renderGameObjects(frameInfo);
		pointLightRenderer.render(frameInfo);
		m_vkRenderer.endSwapChainRenderPass(commandBuffer);//end render pass
		m_vkRenderer.endFrame();
	}
}
