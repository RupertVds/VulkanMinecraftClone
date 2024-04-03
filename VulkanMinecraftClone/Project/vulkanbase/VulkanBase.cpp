#include "vulkanbase/VulkanBase.h"

void VulkanBase::initWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(WIDTH, HEIGHT, "Rupert Vanderstappen", nullptr, nullptr);
}

void VulkanBase::drawFrame(uint32_t imageIndex) 
{
	VkExtent2D swapChainExtent = SwapchainManager::GetInstance().GetSwapchainExtent();

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapChainExtent.width;
	viewport.height = (float)swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(m_CommandBuffer.GetVkCommandBuffer(), 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;
	vkCmdSetScissor(m_CommandBuffer.GetVkCommandBuffer(), 0, 1, &scissor);

	m_RenderPass->Begin(m_CommandBuffer, swapChainFramebuffers, imageIndex);

	m_BasicGraphicsPipeline2D->BindPipeline(m_CommandBuffer.GetVkCommandBuffer());

	m_Scene2D->Render(m_CommandBuffer.GetVkCommandBuffer());

	m_GraphicsPipeline3D->BindPipeline(m_CommandBuffer.GetVkCommandBuffer());
	m_GraphicsPipeline3D->BindDescriptorSets(m_CommandBuffer.GetVkCommandBuffer(), imageIndex);

	m_Scene3D->Render(m_CommandBuffer.GetVkCommandBuffer());

	m_RenderPass->End(m_CommandBuffer);
}

void VulkanBase::createFrameBuffers()
{
	auto& swapChainImageViews = SwapchainManager::GetInstance().GetImageViews();
	swapChainFramebuffers.resize(swapChainImageViews.size());
	for (size_t i = 0; i < swapChainImageViews.size(); i++) 
	{
		VkImageView attachments[] = { swapChainImageViews[i] };

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_RenderPass->GetHandle();
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = SwapchainManager::GetInstance().GetSwapchainExtent().width;
		framebufferInfo.height = SwapchainManager::GetInstance().GetSwapchainExtent().height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}