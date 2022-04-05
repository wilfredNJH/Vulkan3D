#pragma once

#include "camera.hpp"
#include "vk_gameobject.hpp"

// lib
#include <vulkan/vulkan.h>

namespace nekographics {

	#define MAX_LIGHTS 10

	struct PointLight {
		glm::vec4 position{};  // ignore w
		glm::vec4 color{};     // w is intensity
	};

	struct GlobalUbo {
		glm::mat4 projection{ 1.f };
		glm::mat4 view{ 1.f };
		glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .02f };  // w is intensity
		glm::vec4 cameraEyePos{ 0.f,0.f,0.f,1.f};
		PointLight pointLights[MAX_LIGHTS];
		int numLights;
	};

	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;//command buffer can be recorded once and reused for multiple frames 
		NKCamera& camera;
		VkDescriptorSet globalDescriptorSet;
		NkGameObject::Map& gameObjects;
	};
}  // namespace lve
