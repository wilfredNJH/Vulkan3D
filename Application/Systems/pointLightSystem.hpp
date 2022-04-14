#pragma once

#include "camera.hpp"
#include "vk_device.hpp"
#include "vk_frameinfo.hpp"
#include "vk_gameobject.hpp"
#include "vk_pipeline.hpp"

// std
#include <memory>
#include <vector>

namespace nekographics {
    class PointLightSystem {
    public:
        PointLightSystem(
            NKDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~PointLightSystem();

        PointLightSystem(const PointLightSystem&) = delete;
        PointLightSystem& operator=(const PointLightSystem&) = delete;

        void update(FrameInfo& frameInfo, GlobalUbo& ubo);
        void render(FrameInfo& frameInfo);

        bool mFollowCamera = true;//if light is following camera 
        glm::vec4 mStaticCameraPos;//for holding the camera position
    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);


        NKDevice& m_Device;
        std::unique_ptr<NKPipeline> m_Pipeline;
        VkPipelineLayout pipelineLayout;
    };
}  // namespace lve