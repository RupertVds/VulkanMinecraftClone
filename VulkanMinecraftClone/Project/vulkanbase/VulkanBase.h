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
	std::unique_ptr<RenderPass> m_RenderPass;
	//std::unique_ptr<RenderPass> m_RenderPass3D;

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;


	std::unique_ptr<Scene2D> m_Scene2D{};
	std::unique_ptr<Scene> m_Scene3D{};
	std::unique_ptr<BasicGraphicsPipeline2D> m_BasicGraphicsPipeline2D;
	std::unique_ptr<GraphicsPipeline3D> m_GraphicsPipeline3D;

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
		m_RenderPass = std::make_unique<RenderPass>(device);
		//m_RenderPass3D = std::make_unique<RenderPass>(device);
		//createRenderPass();

		m_BasicGraphicsPipeline2D = std::make_unique<BasicGraphicsPipeline2D>(device, m_RenderPass->GetHandle(), "shaders/shader2D.vert.spv",
			"shaders/shader2D.frag.spv");

		m_GraphicsPipeline3D = std::make_unique<GraphicsPipeline3D>(device, physicalDevice, m_RenderPass->GetHandle(), "shaders/shader3D.vert.spv",
			"shaders/shader3D.frag.spv");

		createFrameBuffers();

		// week 02
		m_CommandPool.Initialize(device, QueueManager::GetInstance().FindQueueFamilies(physicalDevice, surface));
		m_CommandBuffer = m_CommandPool.CreateCommandBuffer();

		// week 06
		createSyncObjects();

		m_Scene2D = std::make_unique<Scene2D>(device, physicalDevice, m_CommandPool.GetHandle());

		m_Scene2D->AddTriangle(
			{ {-0.95f, 0.9f}, {1.0f, 0.0f, 0.0f} },
			{ {-0.9f, 0.80f}, {0.0f, 1.0f, 0.0f} },
			{ {-0.85f, 0.9f}, {0.0f, 0.0f, 1.0f} });

		m_Scene2D->AddTriangle(
			{ {-0.80f, 0.9f}, {1.0f, 0.0f, 1.0f} },
			{ {-0.75f, 0.80f}, {1.0f, 1.0f, 0.0f} },
			{ {-0.70f, 0.9f}, {0.0f, 1.0f, 1.0f} });

		m_Scene2D->AddOval({ -0.90f, 0.7f }, 0.05f, 0.05f, 50, { 0.5f, 1.f, 1.f });

		m_Scene2D->AddRectangle(
			{ {-0.80f, 0.65f}, {1.0f, 0.0f, 0.0f} },
			{ {-0.70f, 0.65f}, {0.0f, 1.0f, 0.0f} },
			{ {-0.70f, 0.75f}, {0.0f, 0.0f, 1.0f} },
			{ {-0.80f, 0.75f}, {1.0f, 1.0f, 1.0f} });

		m_Scene3D = std::make_unique<Scene>(device, physicalDevice, m_CommandPool.GetHandle());
		m_Scene3D->AddTriangle(
			{ {-1.f, -1.f, 1.f}, {1.0f, 0.0f, 0.0f} },
			{ {1.f, -1.0f, 1.f}, {0.0f, 1.0f, 0.0f} },
			{ {0.0f, 1.f, 1.f}, {0.0f, 0.0f, 1.0f} });

		m_Scene3D->AddTriangle(
			{ {-0.5f, -0.5f, 2.f}, {0.0f, 1.0f, 0.0f} },
			{ {0.5f, -0.5f, 2.f}, {1.0f, 0.0f, 0.0f} },
			{ {0.0f, 0.5f, 2.f}, {0.0f, 0.0f, 1.0f} });

		m_Scene3D->AddTriangle(
			{ {-0.5f, -0.5f, 3.f}, {0.0f, 1.0f, 0.0f} },
			{ {0.5f, -0.5f, 3.f}, {1.0f, 0.0f, 0.0f} },
			{ {0.0f, 0.5f, 3.f}, {0.0f, 0.0f, 1.0f} });

		m_Scene3D->AddTriangle(
			{ {-0.5f, -0.5f, 4.f}, {0.0f, 1.0f, 0.0f} },
			{ {0.5f, -0.5f, 4.f}, {1.0f, 0.0f, 0.0f} },
			{ {0.0f, 0.5f, 4.f}, {0.0f, 0.0f, 1.0f} });

		m_Scene3D->AddTriangle(
			{ {-0.5f, -0.5f, 5.f}, {0.0f, 1.0f, 0.0f} },
			{ {0.5f, -0.5f, 5.f}, {1.0f, 0.0f, 0.0f} },
			{ {0.0f, 0.5f, 5.f}, {0.0f, 0.0f, 1.0f} });

		m_Scene3D->AddTriangle(
			{ {-0.5f, -0.5f, 6.f}, {0.0f, 1.0f, 0.0f} },
			{ {0.5f, -0.5f, 6.f}, {1.0f, 0.0f, 0.0f} },
			{ {0.0f, 0.5f, 6.f}, {0.0f, 0.0f, 1.0f} });
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
		
		for (auto framebuffer : swapChainFramebuffers)
		{
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}


		m_BasicGraphicsPipeline2D->DestroyPipeline(device);
		m_GraphicsPipeline3D->DestroyPipeline(device);

		m_RenderPass->Destroy(device);
		//m_RenderPass3D->Destroy(device);

		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}
		SwapchainManager::GetInstance().Cleanup();
	
		m_Scene2D->CleanUp();
		m_Scene3D->CleanUp();
		

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