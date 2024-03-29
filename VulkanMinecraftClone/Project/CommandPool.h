#pragma once

#include "vulkan/vulkan_core.h"
#include "vulkanbase/VulkanUtil.h"
#include "CommandBuffer.h"
#include "QueueManager.h"
#include <optional>

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
