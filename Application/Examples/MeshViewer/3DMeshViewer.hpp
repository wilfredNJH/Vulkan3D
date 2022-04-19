/******************************************************************************/
/*!
\file   3DCube.hpp
\author Wilfred Ng Jun Hwee
\par    DP email: junhweewilfred.ng[at]digipen.edu
\par    course: csd2150
\par    Final Mesh Viewer
\date   04/15/2022
\brief
	This is the declaration file of the application
*/
/******************************************************************************/

#pragma once

//includes 
#include "WindowManager.h"
#include "vk_gameobject.hpp"
#include "renderer.hpp"
#include "camera.hpp"
#include "rendererSystem.hpp"
#include "pointLightSystem.hpp"
#include "vk_descriptors.hpp"
#include "vk_texture.hpp"

//std
#include <memory>
#include <vector>

namespace nekographics {

	class gameApp {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		gameApp();
		~gameApp();

		gameApp(const gameApp&) = delete;
		gameApp& operator=(const gameApp&) = delete;

		void pipelineLayout();//setting the pipeline instance 
		void loadTextures(const std::string& textures);//loading the texture 
		void loadPointLights(const int& numberOfLights = 1);//loads the game objects
		void draw(NKCamera& camera, SimpleRenderSystem& renderer, PointLightSystem& pointLightRenderer, FrameInfo& frameInfo,VkCommandBuffer& commandBuffer);//draw call

		VkWindow m_window{ WIDTH,HEIGHT };
		NKDevice  m_vkDevice{ m_window };
		NKRenderer m_vkRenderer{ m_window, m_vkDevice };
		NKTexture m_vktexture{ m_vkDevice };

		// note: order of declarations matters
		std::unique_ptr<NKDescriptorPool> globalPool{};
		std::unique_ptr<NKDescriptorPool> imagePool{};
		NkGameObject::Map gameObjects;//stores the map of gameobjects

		std::vector<std::unique_ptr<nekographics::NKBuffer>> uboBuffers;//vector of ubo buffers
		std::vector<VkDescriptorSet> globalDescriptorSets;//vector of descriptor sets 
		std::unique_ptr<NKDescriptorSetLayout> globalSetLayout;//globalset layout 
	};

}
