#pragma once
#include <vulkan\vulkan_core.h>
#include <stdexcept>
#include <vulkanbase\VulkanUtil.h>

class Texture final
{
public:
	Texture() = default;
	~Texture() = default;
public:
	void Init(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool);
	void Destroy(VkDevice device);

	VkImageView GetImageView() const { return textureImageView; }
	VkSampler GetSampler() const { return textureSampler; }
private:
	void CreateTextureImage(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool);
	void CreateTextureImageView(VkDevice device);
	void CreateTextureSampler(VkDevice device, VkPhysicalDevice physicalDevice);
	void createImage(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
private:
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;
};

