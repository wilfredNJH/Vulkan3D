/******************************************************************************/
/*!
\file   homeworkCube.hpp
\author Wilfred Ng Jun Hwee
\par    DP email: junhweewilfred.ng[at]digipen.edu
\par    course: csd2150
\par    Homework 2
\date   03/07/2022
\brief
	This is the implementation file of the application
*/
/******************************************************************************/

#include "vk_gameobject.hpp"
#include "renderer.hpp"
#include "camera.hpp"
#include "rendererSystem.hpp"
#include "pointLightSystem.hpp"
#include "controller.hpp"
#include "vk_buffer.hpp"
#include "vk_descriptors.hpp"

#include <iostream>
#include "WindowManager.h"
#include "Examples/homeworkCube/3DCube.hpp"



int homeworkCube() {
	
	//creating all vulkan 
	nekographics::gameApp application{};

	//showing the window
	application.m_window.showWindow();

	//creating uniform buffer
	std::vector<std::unique_ptr<nekographics::NKBuffer>> uboBuffers(nekographics::NKSwapChain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < uboBuffers.size(); i++) {
		uboBuffers[i] = std::make_unique<nekographics::NKBuffer>(
			application.m_vkDevice,
			sizeof(nekographics::GlobalUbo),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		uboBuffers[i]->map();
	}

	//creating 

	//descriptor sets
	auto globalSetLayout =
		nekographics::NKDescriptorSetLayout::Builder(application.m_vkDevice)
		.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
		.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
		.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
		.addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
		.addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
		.addBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
		.build();

	std::vector<VkDescriptorSet> globalDescriptorSets(nekographics::NKSwapChain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < globalDescriptorSets.size(); i++) {

		//setting the image info 
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = application.m_vktexture.textureImageViewVec[0];
		imageInfo.sampler = application.m_vktexture.textureSamplerVec[0];

		VkDescriptorImageInfo imageInfo2{};
		imageInfo2.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo2.imageView = application.m_vktexture.textureImageViewVec[1];
		imageInfo2.sampler = application.m_vktexture.textureSamplerVec[1];

		VkDescriptorImageInfo imageInfo3{};
		imageInfo3.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo3.imageView = application.m_vktexture.textureImageViewVec[2];
		imageInfo3.sampler = application.m_vktexture.textureSamplerVec[2];

		VkDescriptorImageInfo imageInfo4{};
		imageInfo4.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo4.imageView = application.m_vktexture.textureImageViewVec[3];
		imageInfo4.sampler = application.m_vktexture.textureSamplerVec[3];

		VkDescriptorImageInfo imageInfo5{};
		imageInfo5.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo5.imageView = application.m_vktexture.textureImageViewVec[4];
		imageInfo5.sampler = application.m_vktexture.textureSamplerVec[4];


		//setting the buffer info 
		auto bufferInfo = uboBuffers[i]->descriptorInfo();
		nekographics::NkDescriptorWriter(*globalSetLayout, *application.globalPool)
			.writeBuffer(0, &bufferInfo)
			.writeImage(1, &imageInfo)
			.writeImage(2, &imageInfo2)
			.writeImage(3, &imageInfo3)
			.writeImage(4, &imageInfo4)
			.writeImage(5, &imageInfo5)
			.build(globalDescriptorSets[i]);
	}

	/***********
	Init Camera & Renderer 
	************/
	nekographics::SimpleRenderSystem simpleRenderSystem{ application.m_vkDevice, application.m_vkRenderer.getSwapChainRenderPass() , globalSetLayout->getDescriptorSetLayout()};
	nekographics::PointLightSystem pointLightSystem{ application.m_vkDevice,application.m_vkRenderer.getSwapChainRenderPass(),globalSetLayout->getDescriptorSetLayout() };

	nekographics::NKCamera camera{};//creating the camera 

	glm::vec3 cameraStartingPosition = { 0.f,0.f,-2.f };
	camera.setViewTarget(cameraStartingPosition, glm::vec3(0.f, 0.f, 0.f));

	//getting the viewer object 
	auto viewerObject = nekographics::NkGameObject::createGameObject();
	viewerObject.transform.translation = cameraStartingPosition;
	nekographics::KeyboardMovementController cameraController{};

	auto currentTime = std::chrono::high_resolution_clock::now();//setting the current time 

	/***********
	Game Loop
	************/
	while (!application.m_window.closeWindow()) {

		/***********
		Input Manager
		************/
		InputManager.update();//update the input manager

		/***********
		Calculating DT
		************/
		auto newTime = std::chrono::high_resolution_clock::now();
		float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();//dt
		currentTime = newTime;//reset current time 

		/***********
		Camera
		************/
		if (cameraController.firstPerson) {
			cameraController.moveInPlaneXZ(&application.m_window, frameTime, viewerObject);//calculating camera controller 
			camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);
		}
		else {
			cameraController.moveThirdPerson(frameTime, application.gameObjects[0],viewerObject);
			camera.setViewTarget(viewerObject.transform.translation, application.gameObjects[0].transform.translation);//setting the view target 
		}

		//checking it's minimised 
		if (!application.m_window.isMinimised()) {
			float aspect = application.m_vkRenderer.getAspectRatio();
			//check if nan
			if (!std::isnan(aspect)) {
				camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);
			}
		}

		//check for window update
		if (!application.m_window.Update()) {
			/***********
			Draw
			************/
			if (application.m_window.mCanRender && !application.m_window.isMinimised()) {
				if (auto commandBuffer = application.m_vkRenderer.beginFrame()) {
					int frameIndex = application.m_vkRenderer.getFrameIndex();
					nekographics::FrameInfo frameInfo{
					  frameIndex,
					  frameTime,
					  commandBuffer,
					  camera,
					  globalDescriptorSets[frameIndex],
					  application.gameObjects };

					// updates
					nekographics::GlobalUbo ubo{};
					ubo.projection = camera.getProjection();
					ubo.view = camera.getView();
					ubo.cameraEyePos = { viewerObject.transform.translation ,1.f };

					pointLightSystem.update(frameInfo, ubo);
					uboBuffers[frameIndex]->writeToBuffer(&ubo);
					uboBuffers[frameIndex]->flush();

					application.draw(camera, simpleRenderSystem,pointLightSystem,frameInfo,commandBuffer);//draw call
				}
			}
			else {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		}
	}

	vkDeviceWaitIdle(application.m_vkDevice.device());//idle device to make sure gpu properly shut down 

	return 0;
}
