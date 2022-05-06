#pragma once

#include "vk_device.hpp"
#include "vk_swapchain.hpp"
#include "WindowManager.h"
#include "vk_frameinfo.hpp"

// std
#include <cassert>
#include <memory>
#include <vector>

//renderer will manage the swap chain, command buffers and the draw frame 1 renderer & multiple rendering system
namespace nekographics {
    class NKRenderer {
    public:
        NKRenderer(VkWindow& window, NKDevice& device);
        ~NKRenderer();

        NKRenderer(const NKRenderer&) = delete;
        NKRenderer& operator=(const NKRenderer&) = delete;

        VkRenderPass getSwapChainRenderPass() const { return m_RendererSwapchain->getRenderPass(); }//getting the swap chain render pass
        float getAspectRatio() const { return m_RendererSwapchain->extentAspectRatio(); }
        bool isFrameInProgress() const { return isFrameStarted; }

        VkCommandBuffer getCurrentCommandBuffer() const {
            assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
            return commandBuffers[currentFrameIndex];
        }

        int getFrameIndex() const {
            assert(isFrameStarted && "Cannot get frame index when frame not in progress");
            return currentFrameIndex;
        }

        VkCommandBuffer beginFrame();
        void endFrame();
        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

        VkImage getDepthImage(int& index) const{ 
            return m_RendererSwapchain->getDepthImages(index);
        }

        VkImageView getDepthView(int& index) const {
            return m_RendererSwapchain->getDepthImageView(index);
        }

        VkSampler getDepthSampler(int& index) const {
            return m_RendererSwapchain->getDepthSampler(index);
        }

    private:
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapChain();

        VkWindow& m_RendererWindow;//reference tothe renderer window 
        NKDevice& m_RendererDevice;//references to the renderer device
        std::unique_ptr<NKSwapChain> m_RendererSwapchain;//renderer swapchain
        std::vector<VkCommandBuffer> commandBuffers;//stores the command buffers

        //tracking the frame 
        uint32_t currentImageIndex;//tracking the frame that is in progress
        int currentFrameIndex{ 0 };//keep track of frame 0 to max frame index in flight 
        bool isFrameStarted{ false };
    };
}  // namespace lve
