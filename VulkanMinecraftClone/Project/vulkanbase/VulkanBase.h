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

const std::vector<const char*> validationLayers = 
{
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = 
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

class VulkanBase {
public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

private:
	GLFWwindow* window;

	CommandPool m_CommandPool;
	CommandBuffer m_CommandBuffer;

	std::vector<VkFramebuffer> swapChainFramebuffers;

	//VkRenderPass renderPass;
	std::unique_ptr<RenderPass> m_RenderPass2D;

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;


	std::unique_ptr<Scene2D> m_Scene2D{};
	std::unique_ptr<BasicGraphicsPipeline2D> m_BasicGraphicsPipeline2D;

	void initVulkan() 
	{
		// week 06
		createInstance();
		setupDebugMessenger();
		createSurface();

		// week 05
		pickPhysicalDevice();
		createLogicalDevice();


		// week 04 
		SwapchainManager::GetInstance().Initialize(instance, physicalDevice, device, surface, window);
		
		// week 03
		//m_MachineShader.Initialize(device);
		m_RenderPass2D = std::make_unique<RenderPass>(device);
		//createRenderPass();

		m_Scene2D = std::make_unique<Scene2D>(device, physicalDevice);

		m_Scene2D->AddTriangle(
			{ {-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f} }, 
			{ {0.5f, -0.5f}, {0.0f, 1.0f, 0.0f} }, 
			{ {0.0f, 0.5f}, {0.0f, 0.0f, 1.0f} });

		m_Scene2D->AddTriangle(
			{ {-1.f, -0.6f}, {1.0f, 1.0f, 0.0f} }, 
			{ {-0.4f, -0.6f}, {0.0f, 1.0f, 1.0f} }, 
			{ {-0.70f, 0.0f}, {1.0f, 0.0f, 1.0f} });

		m_Scene2D->AddTriangle(
			{ {1.f, 0.6f}, {1.0f, 1.0f, 0.0f} },
			{ {0.5f, 0.6f}, {0.0f, 1.0f, 1.0f} },
			{ {0.75f, 0.0f}, {1.0f, 0.0f, 1.0f} });

		m_Scene2D->AddOval({ 0.f,0.f }, 0.3f, 0.3f, 50, {0.f, 0.5f, 1.f});

		m_Scene2D->AddRectangle
		({	{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
			{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}});

		m_BasicGraphicsPipeline2D = std::make_unique<BasicGraphicsPipeline2D>(device, m_RenderPass2D->GetHandle(), "shaders/shader2D.vert.spv",
			"shaders/shader2D.frag.spv");

		createFrameBuffers();

		// week 02
		m_CommandPool.Initialize(device, QueueManager::GetInstance().FindQueueFamilies(physicalDevice, surface));
		m_CommandBuffer = m_CommandPool.CreateCommandBuffer();

		// week 06
		createSyncObjects();
	}

	void mainLoop()
	{
		while (!glfwWindowShouldClose(window)) 
		{
			glfwPollEvents();
			// week 06
			drawFrame();
		}
		vkDeviceWaitIdle(device);
	}

	void cleanup() 
	{
		vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
		vkDestroyFence(device, inFlightFence, nullptr);

		m_CommandPool.Destroy();
		
		for (auto framebuffer : swapChainFramebuffers) {
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}


		m_BasicGraphicsPipeline2D->DestroyPipeline(device);

		m_RenderPass2D->Destroy(device);

		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}
		SwapchainManager::GetInstance().Cleanup();
	
		m_Scene2D->CleanUp();

		vkDestroyDevice(device, nullptr);

		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);

		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void createSurface() 
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface!");
		}
	}

	void initWindow();
	void drawFrame(uint32_t imageIndex);	

	void createFrameBuffers();
	//void createRenderPass();
	
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

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void setupDebugMessenger();
	std::vector<const char*> getRequiredExtensions();
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	void createInstance();

	void createSyncObjects();
	void drawFrame();

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}
};