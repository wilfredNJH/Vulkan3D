#pragma once

#include "camera.hpp"
#include "vk_device.hpp"
#include "vk_gameobject.hpp"
#include "vk_pipeline.hpp"
#include "vk_frameinfo.hpp"
// std
#include <memory>
#include <vector>

namespace nekographics {
	class SimpleRenderSystem {
	public:
		SimpleRenderSystem(NKDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		void renderGameObjects(FrameInfo& frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		NKDevice& m_systemDevice;

		std::unique_ptr<NKPipeline> m_systemPipeline;//pointer to the unique pipeline 
		VkPipelineLayout pipelineLayout;//the pipeline layout 
	};
}  // namespace lve
