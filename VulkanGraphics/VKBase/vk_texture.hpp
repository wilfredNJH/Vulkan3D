#pragma once
#include "WindowManager.h"
#include "vk_device.hpp"
#include <iostream>

namespace nekographics {
	class NKTexture {
	public:
		NKTexture(NKDevice& device);
		~NKTexture();

		//loading of the textures 
		void createTextureImageSTB(const std::string& filepath);
		void createTextureImageDDSMIPMAPS(const std::string& filepath);

		std::vector<VkImage> textureImageVec;
		std::vector<VkDeviceMemory> textureImageMemoryVec;
		std::vector<VkImageView> textureImageViewVec;
		std::vector<VkSampler> textureSamplerVec;
		std::vector<VkDescriptorImageInfo> imageInfoVec;

	private:
		//creating of iamges 
		void createTextureImageView(VkFormat format, uint32_t mipCount = 1);
		void createTextureSampler();
		void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, uint32_t mipCount);
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipCount = 1);
		VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipCount = 1);


		NKDevice& m_vkDevice;//ref to the device 

		//for image
		VkImage textureImage;
		VkDeviceMemory textureImageMemory;
		VkImageView textureImageView;
		VkSampler textureSampler;
	};
}