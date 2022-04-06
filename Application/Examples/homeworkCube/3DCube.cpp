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
		//m_vktexture.createTextureImageDDSMIPMAPS("Textures/dds/Stone Wall 01_1K_Normal - Compress BC5.dds");		//creating DDS image texture for : Normal Map
		//m_vktexture.createTextureImageDDSMIPMAPS("Textures/dds/Stone Wall 01_1K_Diffuse.dds");						//creating DDS image texture for : Diffuse 
		//m_vktexture.createTextureImageDDSMIPMAPS("Textures/dds/Stone Wall 01_1K_Ambient Occlusion.dds");			//creating DDS image texture for : Ambient Occlusion
		//m_vktexture.createTextureImageDDSMIPMAPS("Textures/dds/Stone Wall 01_1K_Glossiness.dds");					//creating DDS image texture for : Glossiness
		//m_vktexture.createTextureImageDDSMIPMAPS("Textures/dds/Stone Wall 01_1K_Roughness.dds");					//creating DDS image texture for : Roughness

		m_vktexture.createTextureImageDDSMIPMAPS("Textures/dds/TD_Checker_Normal_OpenGL.dds");		//creating DDS image texture for : Normal Map
		m_vktexture.createTextureImageDDSMIPMAPS("Textures/dds/TD_Checker_Base_Color.dds");						//creating DDS image texture for : Diffuse 
		m_vktexture.createTextureImageDDSMIPMAPS("Textures/dds/TD_Checker_Mixed_AO.dds");			//creating DDS image texture for : Ambient Occlusion
		m_vktexture.createTextureImageDDSMIPMAPS("Textures/dds/Stone Wall 01_1K_Glossiness.dds");					//creating DDS image texture for : Glossiness
		m_vktexture.createTextureImageDDSMIPMAPS("Textures/dds/TD_Checker_Roughness.dds");					//creating DDS image texture for : Roughness


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
		globalSetLayout =
			nekographics::NKDescriptorSetLayout::Builder(m_vkDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.addBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();


		/**************
		Creating Descriptor Sets
		**************/
		globalDescriptorSets.resize(nekographics::NKSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++) {

			//setting the image info 
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = m_vktexture.textureImageViewVec[0];
			imageInfo.sampler = m_vktexture.textureSamplerVec[0];

			VkDescriptorImageInfo imageInfo2{};
			imageInfo2.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo2.imageView = m_vktexture.textureImageViewVec[1];
			imageInfo2.sampler = m_vktexture.textureSamplerVec[1];

			VkDescriptorImageInfo imageInfo3{};
			imageInfo3.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo3.imageView = m_vktexture.textureImageViewVec[2];
			imageInfo3.sampler = m_vktexture.textureSamplerVec[2];

			VkDescriptorImageInfo imageInfo4{};
			imageInfo4.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo4.imageView = m_vktexture.textureImageViewVec[3];
			imageInfo4.sampler = m_vktexture.textureSamplerVec[3];

			VkDescriptorImageInfo imageInfo5{};
			imageInfo5.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo5.imageView = m_vktexture.textureImageViewVec[4];
			imageInfo5.sampler = m_vktexture.textureSamplerVec[4];


			//setting the buffer info 
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			nekographics::NkDescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.writeImage(1, &imageInfo)
				.writeImage(2, &imageInfo2)
				.writeImage(3, &imageInfo3)
				.writeImage(4, &imageInfo4)
				.writeImage(5, &imageInfo5)
				.build(globalDescriptorSets[i]);
		}
	}

	gameApp::~gameApp() {}

	void gameApp::loadGameObjects() {
		/**************
		Creating Mesh
		**************/
		/*
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
		*/

		/**************
		Creating FBX model 
		**************/
		
		std::shared_ptr<NKModel> lveModel =
			NKModel::createAssimpModelFromFile(m_vkDevice, "Models/FBX/Skull_textured.fbx");
		auto skull = NkGameObject::createGameObject();
		skull.model = lveModel;
		skull.transform.translation = {  0.f, 0.f, 0.f };
		skull.transform.scale = { 0.01, 0.01, 0.01f };
		gameObjects.emplace(skull.getId(), std::move(skull));
		

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
			//{1.f, .1f, .1f},
			//{.1f, .1f, 1.f},
			//{.1f, 1.f, .1f},
			//{1.f, 1.f, .1f},
			//{.1f, 1.f, 1.f}
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
		UNREFERENCED_PARAMETER(camera);

		//check for begin frame 
		m_vkRenderer.beginSwapChainRenderPass(commandBuffer);//begin renderpass
		renderer.renderGameObjects(frameInfo);
		pointLightRenderer.render(frameInfo);
		m_vkRenderer.endSwapChainRenderPass(commandBuffer);//end render pass
		m_vkRenderer.endFrame();
	}
}
