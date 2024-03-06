#include "vulkanbase/VulkanBase.h"

void VulkanBase::initWindow() {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(WIDTH, HEIGHT, "Rupert Vanderstappen", nullptr, nullptr);
}

void VulkanBase::drawScene() {
	// Use mesh draw call here
	vkCmdDraw(m_CommandBuffer.GetVkCommandBuffer(), 6, 1, 0, 0);
}