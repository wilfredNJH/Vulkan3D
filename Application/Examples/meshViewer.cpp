/******************************************************************************/
/*!
\file   meshViewer.hpp
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
#include "vk_gameobject.hpp"
#include "renderer.hpp"
#include "camera.hpp"
#include "rendererSystem.hpp"
#include "pointLightSystem.hpp"
#include "controller.hpp"
#include "vk_buffer.hpp"
#include "vk_descriptors.hpp"
#include "WindowManager.h"
#include "Examples/MeshViewer/3DMeshViewer.hpp"

//std
#include <iostream>


int meshViewer() {
	
	//creating all vulkan 
	nekographics::gameApp application{};

	//showing the window
	application.m_window.showWindow();

	/**************
	Loading Textures
	**************/
	//for the skull
	application.loadTextures("Textures/dds/TD_Checker_Normal_OpenGL.dds");		//loading DDS image texture for : Normal Map
	application.loadTextures("Textures/dds/TD_Checker_Base_Color.dds");			//loading DDS image texture for : Diffuse 
	application.loadTextures("Textures/dds/TD_Checker_Mixed_AO.dds");			//loading DDS image texture for : Ambient Occlusion
	application.loadTextures("Textures/dds/TD_Checker_Roughness.dds");			//loading DDS image texture for : Roughness

	//for the vintage car 
	application.loadTextures("Textures/dds/_Normal_DirectX.dds");				//loading DDS image texture for : Normal Map
	application.loadTextures("Textures/dds/_Base_Color.dds");					//loading DDS image texture for : Diffuse 
	application.loadTextures("Textures/dds/_Mixed_AO.dds");						//loading DDS image texture for : Ambient Occlusion
	application.loadTextures("Textures/dds/_Roughness.dds");					//loading DDS image texture for : Roughness

	application.pipelineLayout();//setting up the pipeline 

	/**************
	Creating FBX model
	**************/
	std::shared_ptr<nekographics::NKModel> skullModel = nekographics::NKModel::createAssimpModelFromFile(application.m_vkDevice, "Models/FBX/Skull_textured.fbx");
	auto skull = nekographics::NkGameObject::createGameObject();
	skull.model = skullModel;
	skull.transform.translation = { 0.f, 0.f, 0.f };
	skull.transform.scale = { 0.01, 0.01, 0.01f };
	application.gameObjects.emplace(skull.getId(), std::move(skull));

	std::shared_ptr<nekographics::NKModel> vintageCarModel = nekographics::NKModel::createAssimpModelFromFile(application.m_vkDevice, "Models/FBX/_2_Vintage_Car_01_low.fbx");
	auto vintageCar = nekographics::NkGameObject::createGameObject();
	vintageCar.model = vintageCarModel;
	vintageCar.transform.translation = { 0.0f, 0.0f, -8.0f };
	vintageCar.transform.scale = { 0.5f, 0.5f, 0.5f };
	application.gameObjects.emplace(vintageCar.getId(), std::move(vintageCar));

	application.loadPointLights(2);//loading point lights

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
			//if you're looking at the first model
			if (cameraController.modelNumber) {
				cameraController.moveThirdPerson(frameTime, application.gameObjects,viewerObject);
				camera.setViewTarget(viewerObject.transform.translation, application.gameObjects[0].transform.translation);//setting the view target 
			}
			else {
				cameraController.moveThirdPerson(frameTime, application.gameObjects, viewerObject);
				camera.setViewTarget(viewerObject.transform.translation, application.gameObjects[1].transform.translation);//setting the view target 
			}
		}

		//checking it's minimised 
		if (!application.m_window.isMinimised()) {
			float aspect = application.m_vkRenderer.getAspectRatio();
			//check if nan
			if (!std::isnan(aspect)) {
				camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);
			}
		}

		pointLightSystem.inputUpdate(viewerObject);//update light with input 
		
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

					pointLightSystem.update(frameInfo, ubo);//point light system update 
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
