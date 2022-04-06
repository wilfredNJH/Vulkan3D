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

	/***********
	Init Camera & Renderer 
	************/
	nekographics::SimpleRenderSystem simpleRenderSystem{ application.m_vkDevice, application.m_vkRenderer.getSwapChainRenderPass() , application.globalSetLayout->getDescriptorSetLayout()};
	nekographics::PointLightSystem pointLightSystem{ application.m_vkDevice,application.m_vkRenderer.getSwapChainRenderPass(),application.globalSetLayout->getDescriptorSetLayout() };

	nekographics::NKCamera camera{};//creating the camera 

	glm::vec3 cameraStartingPosition = { 0.f,0.f,5.f };
	camera.setViewTarget(cameraStartingPosition, glm::vec3(0.f, 0.f, 0.f));

	//getting the viewer object 
	auto viewerObject = nekographics::NkGameObject::createGameObject();
	viewerObject.transform.translation = cameraStartingPosition;
	viewerObject.transform.rotation = { 0.f,3.14f,0.f };//setting the starting rotation of the camera to face obj 
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
					  application.globalDescriptorSets[frameIndex],
					  application.gameObjects };

					// updates
					nekographics::GlobalUbo ubo{};
					ubo.projection = camera.getProjection();
					ubo.view = camera.getView();
					ubo.cameraEyePos = { viewerObject.transform.translation ,1.f };

					pointLightSystem.update(frameInfo, ubo);
					application.uboBuffers[frameIndex]->writeToBuffer(&ubo);
					application.uboBuffers[frameIndex]->flush();

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
