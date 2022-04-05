#include "renderer.hpp"

// std
#include <array>
#include <cassert>
#include <stdexcept>

namespace nekographics {

    NKRenderer::NKRenderer(VkWindow& window, NKDevice& device)
        : m_RendererWindow{ window }, m_RendererDevice{ device } {
        recreateSwapChain();//recreating the swap chain 
        createCommandBuffers();//creating the command buffer 
    }

    NKRenderer::~NKRenderer() { freeCommandBuffers(); }

    void NKRenderer::recreateSwapChain() {
        auto extent = m_RendererWindow.getExtent();
        while (extent.width == 0 || extent.height == 0) {
            extent = m_RendererWindow.getExtent();
            //glfwWaitEvents();
        }
        vkDeviceWaitIdle(m_RendererDevice.device());

        //check if the swap chain is a null pointer
        if (m_RendererSwapchain == nullptr) {
            m_RendererSwapchain = std::make_unique<NKSwapChain>(m_RendererDevice, extent);//create another swap chain 
        }
        else {
            std::shared_ptr<NKSwapChain> oldSwapChain = std::move(m_RendererSwapchain);
            m_RendererSwapchain = std::make_unique<NKSwapChain>(m_RendererDevice, extent, oldSwapChain);//create the new swap chain  

            if (!oldSwapChain->compareSwapFormats(*m_RendererSwapchain.get())) {
                throw std::runtime_error("Swap chain image(or depth) format has changed!");
            }
        }
    }

    void NKRenderer::createCommandBuffers() {
        commandBuffers.resize(NKSwapChain::MAX_FRAMES_IN_FLIGHT);

        //allocate command buffer 
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;//primary vs secondary 
        allocInfo.commandPool = m_RendererDevice.getCommandPool();//getting the command pool
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        //allocate the command buffer 
        if (vkAllocateCommandBuffers(m_RendererDevice.device(), &allocInfo, commandBuffers.data()) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void NKRenderer::freeCommandBuffers() {
        vkFreeCommandBuffers(
            m_RendererDevice.device(),
            m_RendererDevice.getCommandPool(),
            static_cast<uint32_t>(commandBuffers.size()),
            commandBuffers.data());
        commandBuffers.clear();
    }

    VkCommandBuffer NKRenderer::beginFrame() {
        assert(!isFrameStarted && "Can't call beginFrame while already in progress");

        //acquiring the next image 
        auto result = m_RendererSwapchain->acquireNextImage(&currentImageIndex);
        //check if the swap chain is out of date
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            if (!m_RendererWindow.isMinimised()) {
                recreateSwapChain();//recreate the swap chain
                return nullptr;//indicate that the frame is not successful
            }
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        isFrameStarted = true;

        auto commandBuffer = getCurrentCommandBuffer();
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;//setting the command buffer begin info 

        //begin the record the command buffer 
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
        return commandBuffer;
    }

    void NKRenderer::endFrame() {
        assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
        auto commandBuffer = getCurrentCommandBuffer();
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }

        auto result = m_RendererSwapchain->submitCommandBuffers(&commandBuffer, &currentImageIndex);//submitting the comamnd buffer to graphics queue 

        //check if the swap chain is out of date , or it is resized 
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
            m_RendererWindow.wasWindowResize()) {

            if (m_RendererWindow.isMinimised()) {
                m_RendererWindow.resetWindowResizedFlag();
                recreateSwapChain();
            }
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        isFrameStarted = false;
        currentFrameIndex = (currentFrameIndex + 1) % NKSwapChain::MAX_FRAMES_IN_FLIGHT;//setting the frames index 
    }

    void NKRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
        assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
        assert(
            commandBuffer == getCurrentCommandBuffer() &&
            "Can't begin render pass on command buffer from a different frame");

        //begin the render pass 
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_RendererSwapchain->getRenderPass();//getting the renderpass from the swapchain 
        renderPassInfo.framebuffer = m_RendererSwapchain->getFrameBuffer(currentImageIndex);//getting which framebuffer the renderpass is right in 

        //defining the area where the shader loads and store will take place 
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_RendererSwapchain->getSwapChainExtent();

        //what we want the framebuffer to be cleared to, index 0 is the color attachment, index 1 is the depth attachment 
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };//the color to clear 
        clearValues[1].depthStencil = { 1.0f, 0 };
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);//begining the render pass, VK_SUBPASS_CONTENTS_INLINE tells that only primary command buffer is being used

        //setting up the viewport and the scissor 
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_RendererSwapchain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(m_RendererSwapchain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{ {0, 0}, m_RendererSwapchain->getSwapChainExtent() };
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void NKRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
        assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
        assert(
            commandBuffer == getCurrentCommandBuffer() &&
            "Can't end render pass on command buffer from a different frame");
        vkCmdEndRenderPass(commandBuffer);//ending the render pass 
    }

}  // namespace lve
