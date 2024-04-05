#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include "VulkanUtil.h"

#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <set>
#include <limits>
#include <algorithm>
#include <MachineShader.h>
#include <CommandBuffer.h>
#include <CommandPool.h>
#include <Mesh2D.h>
#include <QueueManager.h>
#include <SwapchainManager.h>
#include "Scene2D.h"
#include <BasicGraphicsPipeline2D.h>
#include <RenderPass.h>
#include <GraphicsPipeline3D.h>
#include <Scene.h>
#include <Timer.h>
#include "Camera.h"
#include "InputManager.h"

const std::vector<const char*> validationLayers = 
{
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = 
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

class VulkanBase
{
public:
	void run() 
	{
		initWindow();
		initVulkan();
		mainLoop();
		// TODO: GAME run function
		cleanup();
	}

private:
	GLFWwindow* window;

	CommandPool m_CommandPool;
	CommandBuffer m_CommandBuffer;

	std::unique_ptr<RenderPass> m_RenderPass;

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	// GAME?
	std::unique_ptr<Scene2D> m_Scene2D{};
	std::unique_ptr<Scene> m_Scene3D{};
	std::unique_ptr<BasicGraphicsPipeline2D> m_BasicGraphicsPipeline2D;
	std::unique_ptr<GraphicsPipeline3D> m_GraphicsPipeline3D;

	void initVulkan();
	void initWindow();
	void mainLoop();
	void drawFrame(uint32_t imageIndex);	
	void drawFrame();
	void cleanup();
	void createSurface();

	void pickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
	void createLogicalDevice();

	// Week 06
	// Main initialization
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkDevice device = VK_NULL_HANDLE;
	VkSurfaceKHR surface;

	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
	VkFence inFlightFence;

	void createInstance();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void setupDebugMessenger();
	void createSyncObjects();
	std::vector<const char*> getRequiredExtensions();
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}
};