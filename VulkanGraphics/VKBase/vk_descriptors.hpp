#pragma once

#include "vk_device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace nekographics {

    /*
    1. Descriptors points to a resource
    2. Group descriptors into sets to bind 
    3. DescriptorSetLayouts need to be provided at pipeline creation
    4. Bind descriptor sets before draw call
    5. Descriptor sets can only be created using a descriptor pool object
    */

    class NKDescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(NKDevice& pDevice) : m_DeviceBuilder{ pDevice } {}

            //adding the binds
            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<NKDescriptorSetLayout> build() const;

        private:
            NKDevice& m_DeviceBuilder;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        NKDescriptorSetLayout(
            NKDevice& lveDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~NKDescriptorSetLayout();
        NKDescriptorSetLayout(const NKDescriptorSetLayout&) = delete;
        NKDescriptorSetLayout& operator=(const NKDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
        NKDevice& m_DeviceLayout;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class NkDescriptorWriter;
    };

    class NKDescriptorPool {
    public:
        class Builder {
        public:
            Builder(NKDevice& pDevice) : m_PoolDeviceBuilder{ pDevice } {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);//creating the pool size
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);//total number of allocators
            std::unique_ptr<NKDescriptorPool> build() const;

        private:
            NKDevice& m_PoolDeviceBuilder;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        NKDescriptorPool(
            NKDevice& lveDevice,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~NKDescriptorPool();
        NKDescriptorPool(const NKDescriptorPool&) = delete;
        NKDescriptorPool& operator=(const NKDescriptorPool&) = delete;

        bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void resetPool();

        // getters 
        NKDevice& GetPoolDevice() {
            return m_PoolDevice;
        }

        VkDescriptorPool GetDescriptorPool() {
            return descriptorPool;
        }
    private:
        NKDevice& m_PoolDevice;
        VkDescriptorPool descriptorPool;

        friend class NkDescriptorWriter;
    };

    
    class NkDescriptorWriter {
    public:
        NkDescriptorWriter(NKDescriptorSetLayout& setLayout, NKDescriptorPool& pool);

        NkDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        NkDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        NKDescriptorSetLayout& setLayout;
        NKDescriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

}  // namespace lve
