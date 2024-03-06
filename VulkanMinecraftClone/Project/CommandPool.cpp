#include "CommandPool.h"

CommandPool::CommandPool()
	:
	m_CommandPool{ VK_NULL_HANDLE },
	m_VkDevice{ VK_NULL_HANDLE }
{

}

void CommandPool::Initialize(const VkDevice& device, const QueueFamilyIndices& queue)
{
	m_VkDevice = device;

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queue.graphicsFamily.value();

	if (vkCreateCommandPool(device, &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create command pool!");
	}
}

void CommandPool::Destroy()
{
	vkDestroyCommandPool(m_VkDevice, m_CommandPool, nullptr);
}

CommandBuffer CommandPool::CreateCommandBuffer() const
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_CommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;
		
	VkCommandBuffer commandBuffer;
		
	if (vkAllocateCommandBuffers(m_VkDevice, &allocInfo, &commandBuffer) !=
	VK_SUCCESS) 
	{
		throw std::runtime_error("failed to allocate command buffers!");
	}

	CommandBuffer cmdBuffer;
	cmdBuffer.SetVkCommandBuffer(commandBuffer);
	return cmdBuffer;
}
