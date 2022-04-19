//includes 
#include "vk_texture.hpp"

//libs
#define TINYDDSLOADER_IMPLEMENTATION
#include "tinyddsloader/tinyddsloader.h"
using namespace tinyddsloader;

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"


namespace nekographics {

	VkImageView NKTexture::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipCount) {
		// Create image view
	   // Textures are not directly accessed by the shaders and
	   // are abstracted by image views containing additional
	   // information and sub resource ranges
		VkImageViewCreateInfo viewInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO
		,   .pNext = nullptr
		,   .image = image
		,   .viewType = VK_IMAGE_VIEW_TYPE_2D
		,   .format = format
		,   .components =
			{
				.r = VK_COMPONENT_SWIZZLE_R
			,   .g = VK_COMPONENT_SWIZZLE_G
			,   .b = VK_COMPONENT_SWIZZLE_B
			,   .a = VK_COMPONENT_SWIZZLE_A
			}
		,   .subresourceRange
			{
				.aspectMask = aspectFlags
			,   .baseMipLevel = 0
			// Linear tiling usually won't support mip maps
			// Only set mip map count if optimal tiling is used
		,   .levelCount = static_cast<std::uint32_t>(mipCount)
		,   .baseArrayLayer = 0
		,   .layerCount = 1
		}
		};

		VkImageView imageView;
		if (vkCreateImageView(m_vkDevice.device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}

		return imageView;
	}

	void NKTexture::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipCount) {
		UNREFERENCED_PARAMETER(format);
		VkCommandBuffer commandBuffer = m_vkDevice.beginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipCount;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		m_vkDevice.endSingleTimeCommands(commandBuffer);
	}

	void NKTexture::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, uint32_t mipCount) {
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipCount;//needs to change
		imageInfo.arrayLayers = 1;//
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(m_vkDevice.device(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(m_vkDevice.device(), image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = m_vkDevice.findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(m_vkDevice.device(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(m_vkDevice.device(), image, imageMemory, 0);
	}

	void NKTexture::createTextureImageSTB(const std::string& texturePath) {
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
		}

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		m_vkDevice.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(m_vkDevice.device(), stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(m_vkDevice.device(), stagingBufferMemory);

		stbi_image_free(pixels);

		createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory, 1);

		transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		m_vkDevice.copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);
		transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(m_vkDevice.device(), stagingBuffer, nullptr);
		vkFreeMemory(m_vkDevice.device(), stagingBufferMemory, nullptr);

		//storing into a vector 
		textureImageMemoryVec.emplace_back(textureImageMemory);
		textureImageVec.emplace_back(textureImage);

		//createTextureImageView();
		//createTextureSampler();
	}

	void NKTexture::createTextureImageView(VkFormat format, uint32_t mipCount) {
		textureImageView = createImageView(textureImage, format, VK_IMAGE_ASPECT_COLOR_BIT, mipCount);
		textureImageViewVec.emplace_back(textureImageView);//storing into vec 
	}

	void NKTexture::createTextureSampler() {
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(m_vkDevice.getPhysicalDevice(), &properties);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.maxLod = 1000;

		if (vkCreateSampler(m_vkDevice.device(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}

		//storing into vec 
		textureSamplerVec.emplace_back(textureSampler);
	}


	NKTexture::NKTexture(NKDevice& device) : m_vkDevice(device){
	}

	NKTexture::~NKTexture(){
		for (auto& x : textureSamplerVec) {
			vkDestroySampler(m_vkDevice.device(), x, nullptr);
		}
		for (auto& x : textureImageViewVec) {
			vkDestroyImageView(m_vkDevice.device(), x, nullptr);
		}
		for (auto& x : textureImageVec) {
			vkDestroyImage(m_vkDevice.device(), x, nullptr);
		}
		for (auto& x : textureImageMemoryVec) {
			vkFreeMemory(m_vkDevice.device(), x, nullptr);
		}

	}

	void NKTexture::createTextureImageDDS(const std::string& texturePath) {

		DDSFile dds;
		auto ret = dds.Load(texturePath.c_str());
		if (tinyddsloader::Result::Success != ret) {
			std::cout << "Failed to load.\n";
			std::cout << "Result : " << int(ret) << "\n";
		}
		VkDeviceSize imageSize = dds.GetImageData(0, 0)->m_memSlicePitch;//getting the image size of 1 image(mip map)

		void* pixels = dds.GetImageData(0, 0)->m_mem;
		int texWidth, texHeight;
		//setting the width and the height of the image 
		texWidth = dds.GetWidth();
		texHeight = dds.GetHeight();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		m_vkDevice.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(m_vkDevice.device(), stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(m_vkDevice.device(), stagingBufferMemory);

		createImage(texWidth, texHeight, VK_FORMAT_BC1_RGBA_SRGB_BLOCK, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory, dds.GetMipCount());

		transitionImageLayout(textureImage, VK_FORMAT_BC1_RGBA_SRGB_BLOCK, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		m_vkDevice.copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);
		transitionImageLayout(textureImage, VK_FORMAT_BC1_RGBA_SRGB_BLOCK, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(m_vkDevice.device(), stagingBuffer, nullptr);
		vkFreeMemory(m_vkDevice.device(), stagingBufferMemory, nullptr);

		//storing into a vector 
		textureImageMemoryVec.emplace_back(textureImageMemory);
		textureImageVec.emplace_back(textureImage);

		//createTextureImageView();
		//createTextureSampler();
	}

	void NKTexture::createTextureImageDDSMIPMAPS(const std::string& texturePath) {

		DDSFile dds;
		auto ret = dds.Load(texturePath.c_str());
		if (tinyddsloader::Result::Success != ret) {
			std::cout << "Failed to load.\n";
			std::cout << "Result : " << int(ret) << "\n";
		}

		//make sure we can handle the texture 
		if (dds.GetTextureDimension() == DDSFile::TextureDimension::Unknown
			|| dds.GetTextureDimension() == DDSFile::TextureDimension::Texture3D)
		{
			std::cout << "Unsupported dimension of texture\n";
		}


		//getting basic information about the texture 
		VkFormat ddsVKFormat;//stores the vk format 
		//getting the format of the dds 
		// https://www.khronos.org/registry/vulkan/specs/1.0/xhtml/vkspec.html 
		// VK Interpretation of Numeric Format
		// UNORM   - The components are unsigned normalized values in the range [0,1]
		// SNORM   - The components are signed normalized values in the range [-1,1]
		// USCALED - The components are unsigned integer values that get converted to floating-point in the range [0,2n-1]
		// SSCALED - The components are signed integer values that get converted to floating-point in the range [-2n-1,2n-1-1]
		// UINT    - The components are unsigned integer values in the range [0,2n-1]
		// SINT    - The components are signed integer values in the range [-2n-1,2n-1-1]
		// UFLOAT  - The components are unsigned floating-point numbers (used by packed, shared exponent, and some compressed formats)
		// SFLOAT  - The components are signed floating-point numbers
		// SRGB    - The R, G, and B components are unsigned normalized values that represent values using sRGB nonlinear encoding, 
		//           while the A component (if one exists) is a regular unsigned normalized value
		switch (dds.GetFormat()) {
		case DDSFile::DXGIFormat::BC1_UNorm:
			ddsVKFormat = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
			break;
		case DDSFile::DXGIFormat::BC1_UNorm_SRGB:
			ddsVKFormat = VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
			break;
		case DDSFile::DXGIFormat::BC2_UNorm:
			ddsVKFormat = VK_FORMAT_BC2_UNORM_BLOCK;
			break;
		case DDSFile::DXGIFormat::BC2_UNorm_SRGB:
			ddsVKFormat = VK_FORMAT_BC2_SRGB_BLOCK;
			break;
		case DDSFile::DXGIFormat::BC3_UNorm:
			ddsVKFormat = VK_FORMAT_BC3_UNORM_BLOCK;
			break;
		case DDSFile::DXGIFormat::BC3_UNorm_SRGB:
			ddsVKFormat = VK_FORMAT_BC3_SRGB_BLOCK;
			break;
		case DDSFile::DXGIFormat::R8G8B8A8_UNorm:
			ddsVKFormat = VK_FORMAT_R8G8B8A8_UNORM;
			break;
		case DDSFile::DXGIFormat::R8G8B8A8_UNorm_SRGB:
			ddsVKFormat = VK_FORMAT_R8G8B8A8_SRGB;
			break;
		case DDSFile::DXGIFormat::R8G8B8A8_SNorm:
			ddsVKFormat = VK_FORMAT_R8G8B8A8_SNORM;
			break;
		case DDSFile::DXGIFormat::B8G8R8A8_UNorm:
			ddsVKFormat = VK_FORMAT_B8G8R8A8_UNORM;
			break;
		case DDSFile::DXGIFormat::B8G8R8A8_UNorm_SRGB:
			ddsVKFormat = VK_FORMAT_B8G8R8A8_SRGB;
			break;
		case DDSFile::DXGIFormat::B8G8R8A8_Typeless:
			ddsVKFormat = VK_FORMAT_B8G8R8A8_SNORM;
			break;
		case DDSFile::DXGIFormat::BC5_UNorm:
			ddsVKFormat = VK_FORMAT_BC5_UNORM_BLOCK;
			break;
		case DDSFile::DXGIFormat::BC5_SNorm:
			ddsVKFormat = VK_FORMAT_BC5_SNORM_BLOCK;
			break;
		default:
			ddsVKFormat = VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
		}

		const auto TotalByteSize = [&]
		{
			auto FaceByteSize = 0;

			// Mips should be organized from biggest to smallest
			std::uint32_t PrevW = 0xffffffff;
			for (std::uint32_t i = 0; i < dds.GetMipCount(); i++)
			{
				auto View = dds.GetImageData(i, 0);
				FaceByteSize += View->m_memSlicePitch;
				if (View->m_width >= PrevW)
				{
					return -1;
				}
				PrevW = View->m_width;
			}

			return FaceByteSize;
		}();

		////
		//// Copy memory
		////
		VkDeviceSize imageSize = TotalByteSize;//getting the image size of the entire mipmaps and layers 

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		m_vkDevice.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		std::vector<uint32_t> offsetVector;//stores the offsets 

		void* data;
		vkMapMemory(m_vkDevice.device(), stagingBufferMemory, 0, imageSize, 0, &data);
		{
			auto pMemory = reinterpret_cast<std::byte*>(data);
			int  Offset = 0;
			//auto TopMipView = dds.GetImageData(0, 0);
			for (std::uint32_t iMip = 0; iMip < dds.GetMipCount(); ++iMip)
			{
				auto View = dds.GetImageData(iMip, 0);
				auto ByteSize = View->m_memSlicePitch;

				// Copy the mip data
				std::memcpy(&pMemory[Offset], View->m_mem, ByteSize);
				Offset += ByteSize;
				offsetVector.emplace_back(ByteSize);
			}
		}
		vkUnmapMemory(m_vkDevice.device(), stagingBufferMemory);

		int texWidth, texHeight;
		//setting the width and the height of the image 
		texWidth = dds.GetWidth();
		texHeight = dds.GetHeight();

		createImage(texWidth, texHeight, ddsVKFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory, dds.GetMipCount());

		transitionImageLayout(textureImage, ddsVKFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, dds.GetMipCount());
		m_vkDevice.copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1, dds.GetMipCount(), offsetVector);
		transitionImageLayout(textureImage, ddsVKFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, dds.GetMipCount());

		vkDestroyBuffer(m_vkDevice.device(), stagingBuffer, nullptr);
		vkFreeMemory(m_vkDevice.device(), stagingBufferMemory, nullptr);

		//storing into a vector 
		textureImageMemoryVec.emplace_back(textureImageMemory);
		textureImageVec.emplace_back(textureImage);

		createTextureImageView(ddsVKFormat, dds.GetMipCount());
		createTextureSampler();
	}
}