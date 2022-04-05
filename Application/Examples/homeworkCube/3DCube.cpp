/******************************************************************************/
/*!
\file   3DCube.cpp
\author Wilfred Ng Jun Hwee
\par    DP email: junhweewilfred.ng[at]digipen.edu
\par    course: csd2150
\par    Homework 2
\date   03/07/2022
\brief
	This is the implementation file of the application
*/
/******************************************************************************/

//includes
#include "3DCube.hpp"
#include "controller.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

//std
#include <chrono>
#include <iostream>

namespace nekographics {

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
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, NKSwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();


		loadGameObjects();//loading in the game objects

		/**************
		Creating Textures
		**************/
		//m_vktexture.createTextureImageSTB("Textures/Stone Wall 01_1K_Diffuse.png");//for the stb
		//m_vktexture.createTextureImageDDS("Textures/dds/Stone Wall 01_1K_Diffuse.dds");//for the dds 
		m_vktexture.createTextureImageDDSMIPMAPS("Textures/dds/Stone Wall 01_1K_Normal - Compress BC5.dds");//testing for the mipmaps
		m_vktexture.createTextureImageDDSMIPMAPS("Textures/dds/Stone Wall 01_1K_Diffuse.dds");//testing for the mipmaps
		m_vktexture.createTextureImageDDSMIPMAPS("Textures/dds/Stone Wall 01_1K_Ambient Occlusion.dds");//testing for the mipmaps
		m_vktexture.createTextureImageDDSMIPMAPS("Textures/dds/Stone Wall 01_1K_Glossiness.dds");//testing for the mipmaps
		m_vktexture.createTextureImageDDSMIPMAPS("Textures/dds/Stone Wall 01_1K_Roughness.dds");//testing for the mipmaps
	}

	gameApp::~gameApp() {}

	void gameApp::loadGameObjects() {
		/**************
		Creating Mesh
		**************/
		auto Mesh =  //xprim_geom::uvsphere::Generate( 30, 30, 2, 1 ); glm::vec2 UVScale{4,4};
			//xprim_geom::capsule::Generate(30, 30, 1, 4); glm::vec2 UVScale{ 3,3 };
			xprim_geom::cube::Generate(4, 4, 4, 4, xprim_geom::float3{ 1,1,1 }); glm::vec2 UVScale{ 1,1 };
		std::shared_ptr<NKModel> lveModel =
			NKModel::processMesh(m_vkDevice, Mesh);
		auto meshTest = NkGameObject::createGameObject();
		meshTest.model = lveModel;
		meshTest.transform.translation = { 0.f, 0.f, 0.f };
		meshTest.transform.scale = { 0.5, 0.5, 0.5f };
		gameObjects.emplace(meshTest.getId(), std::move(meshTest));

		/**************
		Creating FBX model 
		**************/
		/*
		std::shared_ptr<NKModel> lveModel =
			NKModel::createAssimpModelFromFile(m_vkDevice, "Models/FBX/BaseMesh_Anim.fbx");
		auto flatVase = NkGameObject::createGameObject();
		flatVase.model = lveModel;
		flatVase.transform.translation = {  0.f, 0.f, 0.f };
		flatVase.transform.scale = { 0.1, 0.1, 0.1f };
		gameObjects.emplace(flatVase.getId(), std::move(flatVase));
		*/

		/**************
		Creating OBJ model
		**************/
		/*
		std::shared_ptr<NKModel> lveModelCubeObj =
			NKModel::createModelFromFile(m_vkDevice, "Models/OBJ/cube.obj");
		auto flatVase = NkGameObject::createGameObject();
		flatVase.model = lveModelCubeObj;
		flatVase.transform.translation = { 0.f, 0.f, 0.f };
		flatVase.transform.scale = { 0.5, 0.5, 0.5f };
		gameObjects.emplace(flatVase.getId(), std::move(flatVase));
		*/
		
		/*
		lveModel = NKModel::createModelFromFile(m_vkDevice, "Models/OBJ/smooth_vase.obj");
		auto smoothVase = NkGameObject::createGameObject();
		smoothVase.model = lveModel;
		smoothVase.transform.translation = { .5f, .5f, 0.f };
		smoothVase.transform.scale = { 3.f, 1.5f, 3.f };
		gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));
		*/

		/*
		lveModel = NKModel::createModelFromFile(m_vkDevice, "Models/OBJ/quad.obj");
		auto floor = NkGameObject::createGameObject();
		floor.model = lveModel;
		floor.transform.translation = { 0.f, 1.f, 0.f };
		floor.transform.scale = { 3.f, 1.f, 3.f };
		gameObjects.emplace(floor.getId(), std::move(floor));
		*/

		std::vector<glm::vec3> lightColors{
			{1.f, 1.f, 1.f},
			{1.f, .1f, .1f},
			{.1f, .1f, 1.f},
			{.1f, 1.f, .1f},
			{1.f, 1.f, .1f},
			{.1f, 1.f, 1.f}
		};

		for (int i = 0; i < lightColors.size(); i++) {
			auto pointLight = NkGameObject::makePointLight(1.f);
			pointLight.color = lightColors[i];
			auto rotateLight = glm::rotate(
				glm::mat4(1.f),
				(i * glm::two_pi<float>()) / lightColors.size(),
				{ 0.f, -1.f, 0.f });
			pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
			gameObjects.emplace(pointLight.getId(), std::move(pointLight));
		}
	}

	void gameApp::draw(NKCamera& camera, SimpleRenderSystem& renderer , PointLightSystem& pointLightRenderer,FrameInfo& frameInfo, VkCommandBuffer& commandBuffer) {
		//check for begin frame 
		m_vkRenderer.beginSwapChainRenderPass(commandBuffer);//begin renderpass
		renderer.renderGameObjects(frameInfo);
		pointLightRenderer.render(frameInfo);
		m_vkRenderer.endSwapChainRenderPass(commandBuffer);//end render pass
		m_vkRenderer.endFrame();
	}
}
