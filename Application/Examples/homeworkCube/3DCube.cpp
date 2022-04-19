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
			
			//for the vintage textures 
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, NKSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, NKSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, NKSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, NKSwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();


		loadGameObjects();//loading in the game objects

		/**************
		Creating Textures
		**************/
		//for the skull
		m_vktexture.createTextureImageDDSMIPMAPS("Textures/dds/TD_Checker_Normal_OpenGL.dds");		//creating DDS image texture for : Normal Map
		m_vktexture.createTextureImageDDSMIPMAPS("Textures/dds/TD_Checker_Base_Color.dds");			//creating DDS image texture for : Diffuse 
		m_vktexture.createTextureImageDDSMIPMAPS("Textures/dds/TD_Checker_Mixed_AO.dds");			//creating DDS image texture for : Ambient Occlusion
		m_vktexture.createTextureImageDDSMIPMAPS("Textures/dds/TD_Checker_Roughness.dds");			//creating DDS image texture for : Roughness

		//for the vintage car 
		m_vktexture.createTextureImageDDSMIPMAPS("Textures/dds/_Normal_DirectX.dds");				//creating DDS image texture for : Normal Map
		m_vktexture.createTextureImageDDSMIPMAPS("Textures/dds/_Base_Color.dds");					//creating DDS image texture for : Diffuse 
		m_vktexture.createTextureImageDDSMIPMAPS("Textures/dds/_Mixed_AO.dds");						//creating DDS image texture for : Ambient Occlusion
		m_vktexture.createTextureImageDDSMIPMAPS("Textures/dds/_Roughness.dds");					//creating DDS image texture for : Roughness


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

	gameApp::~gameApp() {}

	void gameApp::loadGameObjects() {

		/**************
		Creating FBX model 
		**************/
		
		std::shared_ptr<NKModel> skullModel =
			NKModel::createAssimpModelFromFile(m_vkDevice, "Models/FBX/Skull_textured.fbx");
		auto skull = NkGameObject::createGameObject();
		skull.model = skullModel;
		skull.transform.translation = {  0.f, 0.f, 0.f };
		skull.transform.scale = { 0.01, 0.01, 0.01f };
		gameObjects.emplace(skull.getId(), std::move(skull));

		std::shared_ptr<NKModel> vintageCarModel =
			NKModel::createAssimpModelFromFile(m_vkDevice, "Models/FBX/_2_Vintage_Car_01_low.fbx");
		auto vintageCar = NkGameObject::createGameObject();
		vintageCar.model = vintageCarModel;
		vintageCar.transform.translation = { 0.0f, 0.0f, -4.0f };
		vintageCar.transform.scale = { 0.5f, 0.5f, 0.5f };
		gameObjects.emplace(vintageCar.getId(), std::move(vintageCar));

		/**************
		Creating Mesh
		**************/
		
		//auto Mesh =  //xprim_geom::uvsphere::Generate( 30, 30, 2, 1 ); glm::vec2 UVScale{4,4};
		//	xprim_geom::capsule::Generate(30, 30, 1, 4); glm::vec2 UVScale{ 3,3 };
		//	//xprim_geom::cube::Generate(4, 4, 4, 4, xprim_geom::float3{ 1,1,1 }); glm::vec2 UVScale{ 1,1 };
		//std::shared_ptr<NKModel> customModel =
		//	NKModel::processMesh(m_vkDevice, Mesh);
		//auto customMesh = NkGameObject::createGameObject();
		//customMesh.model = customModel;
		//customMesh.transform.translation = { -3.f, 0.f, 0.f };
		//customMesh.transform.scale = { 0.5, 0.5, 0.5f };
		//gameObjects.emplace(customMesh.getId(), std::move(customMesh));
		

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
		};

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
