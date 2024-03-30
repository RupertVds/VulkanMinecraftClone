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
	const std::vector<Vertex2D> verticesTriangleOne = 
	{
	{{0.0f, -0.5f}, {0.5f, 1.0f, 1.0f}},
	{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
	{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
	};

	std::unique_ptr<Scene2D> m_Scene2D{};

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
		m_MachineShader.Initialize(device);
		createRenderPass();

		m_Scene2D = std::make_unique<Scene2D>(device, physicalDevice);

		Vertex2D v1{ {-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f} };
		Vertex2D v2{ {0.5f, -0.5f} , {0.0f, 1.0f, 0.0f} };
		Vertex2D v3{ {0.0f, 0.5f}  , {0.0f, 0.0f, 1.0f} };

		m_Scene2D->AddTriangle(v1, v2, v3);

		Vertex2D v4{ {-1.f, -0.6f}, {1.0f, 1.0f, 0.0f} };  
		Vertex2D v5{ {-0.4f, -0.6f} , {0.0f, 1.0f, 1.0f} };
		Vertex2D v6{ {-0.70f, 0.0f}  , {1.0f, 0.0f, 1.0f} };

		m_Scene2D->AddTriangle(v4, v5, v6);

		Vertex2D v7{ {1.f, 0.6f}, {1.0f, 1.0f, 0.0f} };
		Vertex2D v8{ {0.5f, 0.6f} , {0.0f, 1.0f, 1.0f} };
		Vertex2D v9{ {0.75f, 0.0f}  , {1.0f, 0.0f, 1.0f} };

		m_Scene2D->AddTriangle(v7, v8, v9);

		createGraphicsPipeline();
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

		//vkDestroyCommandPool(device, m_CommandPool, nullptr);
		m_CommandPool.Destroy();
		

		for (auto framebuffer : swapChainFramebuffers) {
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}

		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyRenderPass(device, renderPass, nullptr);

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

	// Week 01: 
	// Actual window
	// simple fragment + vertex shader creation functions
	// These 5 functions should be refactored into a separate C++ class
	// with the correct internal state.

	GLFWwindow* window;
	// important to initialize before creating the graphics pipeline
	MachineShader m_MachineShader{
		"shaders/shader.vert.spv", 
		"shaders/shader.frag.spv" 
	};
	void initWindow();
	void drawScene();

	// Week 02
	// Queue families
	// CommandBuffer concept
	// 
	// ===========================
	CommandPool m_CommandPool;
	// ===========================
	CommandBuffer m_CommandBuffer;
	// ===========================

	void drawFrame(uint32_t imageIndex);
	
	// Week 03
	// Renderpass concept
	// Graphics pipeline
	
	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	VkRenderPass renderPass;

	void createFrameBuffers();
	void createRenderPass();
	void createGraphicsPipeline();

	// Week 04
	// Swap chain and image view support

	// Week 05 
	// Logical and physical device

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	//VkQueue graphicsQueue;
	//VkQueue presentQueue;
	
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