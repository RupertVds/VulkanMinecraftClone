#pragma once

#include "vulkan/vulkan_core.h"
#include "vulkanbase/VulkanUtil.h"
#include "CommandBuffer.h"
#include <optional>

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};


class CommandPool final
{
public:
	CommandPool();
	~CommandPool() = default;

	void Initialize(const VkDevice& device, const QueueFamilyIndices& queue);
	void Destroy();

	CommandBuffer CreateCommandBuffer() const;
private:
	VkCommandPool m_CommandPool;
	VkDevice m_VkDevice;
};
