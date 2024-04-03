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
	VkCommandPool GetHandle() { return m_CommandPool; }
	void Destroy();

	CommandBuffer CreateCommandBuffer() const;
private:
	VkCommandPool m_CommandPool;
	//TODO: work this device away maybe
	VkDevice m_VkDevice;
};
