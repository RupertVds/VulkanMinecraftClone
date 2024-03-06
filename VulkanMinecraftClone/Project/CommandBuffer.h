#pragma once

#include "vulkan/vulkan_core.h"
#include "vulkanbase/VulkanUtil.h"

class CommandBuffer final
{
public:
	CommandBuffer();
	~CommandBuffer() = default;

	void SetVkCommandBuffer(VkCommandBuffer buffer) 
	{
		 m_CommandBuffer = buffer;
	}
	
	VkCommandBuffer GetVkCommandBuffer()const 
	{
		return m_CommandBuffer;
	}
	
	void Reset()const;
	void BeginRecording()const;
	void EndRecording()const;
	void Submit(VkSubmitInfo & info)const;
private:
	VkCommandBuffer m_CommandBuffer;
};

