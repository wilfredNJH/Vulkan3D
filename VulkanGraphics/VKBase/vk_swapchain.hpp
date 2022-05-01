#pragma once

#include "vk_device.hpp"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <string>
#include <vector>
#include <memory>

namespace nekographics {
    //coordinates to swap the back and front buffer 

    /*
    1. we will have multiple framebuffers (2 or 3)

    2. we can use swapChain.acquireNextImage() to get the next index of our framebuffers
    */
    class NKSwapChain {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        NKSwapChain(NKDevice& deviceRef, VkExtent2D windowExtent);
        NKSwapChain(NKDevice& deviceRef, VkExtent2D windowExtent, std::shared_ptr<NKSwapChain> previous);
        ~NKSwapChain();

        NKSwapChain(const NKSwapChain&) = delete;
        NKSwapChain& operator=(const NKSwapChain&) = delete;

        VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
        VkRenderPass getRenderPass() { return renderPass; }
        VkImageView getImageView(int index) { return swapChainImageViews[index]; }
        size_t imageCount() { return swapChainImages.size(); }
        VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
        VkExtent2D getSwapChainExtent() { return swapChainExtent; }
        uint32_t width() { return swapChainExtent.width; }
        uint32_t height() { return swapChainExtent.height; }

        float extentAspectRatio() {
            return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
        }
        VkFormat findDepthFormat();

        VkResult acquireNextImage(uint32_t* imageIndex);
        VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

        bool compareSwapFormats(const NKSwapChain& pswapChain) const {
            return pswapChain.swapChainDepthFormat == swapChainDepthFormat &&
                pswapChain.swapChainImageFormat == swapChainImageFormat;
        }

    private:
        void init();
        void createSwapChain();
        void createImageViews();
        void createDepthResources();
        void createRenderPass();
        void createFramebuffers();
        void createSyncObjects();

        // Helper functions
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(
            const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        VkFormat swapChainImageFormat;//keeping track of the image format 
        VkFormat swapChainDepthFormat;//keeping track of the depth format 
        VkExtent2D swapChainExtent;

        std::vector<VkFramebuffer> swapChainFramebuffers;//stores all the framebuffer objects, color/depth
        VkRenderPass renderPass;

        std::vector<VkImage> depthImages;
        std::vector<VkSampler> depthSampler;
        std::vector<VkDeviceMemory> depthImageMemorys;
        std::vector<VkImageView> depthImageViews;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;

        NKDevice& device;
        VkExtent2D windowExtent;

        VkSwapchainKHR swapChain;
        std::shared_ptr<NKSwapChain> oldSwapChain;//pointer to the old swapchain

        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
        std::vector<VkFence> imagesInFlight;
        size_t currentFrame = 0;
    };

}  // namespace lve
