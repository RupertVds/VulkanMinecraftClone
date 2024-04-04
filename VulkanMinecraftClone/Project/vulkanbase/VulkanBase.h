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

		m_Scene2D->AddOval({ -0.90f, 0.7f }, 0.05f, 0.05f, 50, { 0.25f, 1.f, 0.25f });

		m_Scene2D->AddRectangle(
			{ {-0.80f, 0.65f}, {1.0f, 0.0f, 0.0f} },
			{ {-0.70f, 0.65f}, {0.0f, 1.0f, 0.0f} },
			{ {-0.70f, 0.75f}, {0.0f, 0.0f, 1.0f} },
			{ {-0.80f, 0.75f}, {1.0f, 1.0f, 1.0f} });

		m_Scene3D = std::make_unique<Scene>(device, physicalDevice, m_CommandPool.GetHandle());
		// Define the vertices for the cube
		std::vector<Vertex> vertices = {
			// Front face
			{{-1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},   // 0: Bottom-left
			{{1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},    // 1: Bottom-right
			{{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},     // 2: Top-right
			{{-1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.0f}},    // 3: Top-left

			// Back face
			{{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 1.0f}},  // 4: Bottom-left
			{{1.0f, -1.0f, -1.0f}, {0.0f, 1.0f, 1.0f}},   // 5: Bottom-right
			{{1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}},    // 6: Top-right
			{{-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}},   // 7: Top-left
		};

		// Define the indices for the cube
		std::vector<uint16_t> indices = {
			// Front face
			0, 1, 2,  // Triangle 1
			2, 3, 0,  // Triangle 2

			// Right face
			1, 5, 6,  // Triangle 1
			6, 2, 1,  // Triangle 2

			// Back face
			7, 6, 5,  // Triangle 1
			5, 4, 7,  // Triangle 2

			// Left face
			4, 0, 3,  // Triangle 1
			3, 7, 4,  // Triangle 2

			// Top face
			3, 2, 6,  // Triangle 1
			6, 7, 3,  // Triangle 2

			// Bottom face
			4, 5, 1,  // Triangle 1
			1, 0, 4   // Triangle 2
		};

		// Add the cube to the scene
		m_Scene3D->AddTriangle({ 0, 0, 0 }, vertices[0], vertices[1], vertices[2]); // Front face
		m_Scene3D->AddTriangle({ 0, 0, 0 }, vertices[0], vertices[2], vertices[3]);
		m_Scene3D->AddTriangle({ 0, 0, 0 }, vertices[1], vertices[5], vertices[6]); // Right face
		m_Scene3D->AddTriangle({ 0, 0, 0 }, vertices[1], vertices[6], vertices[2]);
		m_Scene3D->AddTriangle({ 0, 0, 0 }, vertices[7], vertices[6], vertices[5]); // Back face
		m_Scene3D->AddTriangle({ 0, 0, 0 }, vertices[7], vertices[5], vertices[4]);
		m_Scene3D->AddTriangle({ 0, 0, 0 }, vertices[4], vertices[0], vertices[3]); // Left face
		m_Scene3D->AddTriangle({ 0, 0, 0 }, vertices[4], vertices[3], vertices[7]);
		m_Scene3D->AddTriangle({ 0, 0, 0 }, vertices[3], vertices[2], vertices[6]); // Top face
		m_Scene3D->AddTriangle({ 0, 0, 0 }, vertices[3], vertices[6], vertices[7]);
		m_Scene3D->AddTriangle({ 0, 0, 0 }, vertices[4], vertices[1], vertices[0]); // Bottom face
		m_Scene3D->AddTriangle({ 0, 0, 0 }, vertices[4], vertices[5], vertices[1]);

	}

	void mainLoop()
	{
		float printTimer = 0.f;
		Timer::GetInstance().Start();
		InputManager::GetInstance().Init(window);
		Camera::GetInstance().Init(&InputManager::GetInstance(), {0, 0, 5});

		while (!glfwWindowShouldClose(window)) 
		{
			glfwPollEvents();
			Timer::GetInstance().Update();
			printTimer += Timer::GetInstance().GetElapsed();
			if (printTimer >= 1.f)
			{
				printTimer = 0.f;
				std::cout << "dFPS: " << Timer::GetInstance().GetdFPS() << std::endl;
			}
			Camera::GetInstance().Update(Timer::GetInstance().GetElapsed());
			if (InputManager::GetInstance().IsKeyPressed(GLFW_KEY_C))
			{
				InputManager::GetInstance().ToggleFPSMode();
			}

			// week 06
			drawFrame();
		}
		vkDeviceWaitIdle(device);
		Timer::GetInstance().Stop();
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
	void keyEvent(int key, int scancode, int action, int mods);
	void mouseMove(GLFWwindow* window, double xpos, double ypos);
	void mouseEvent(GLFWwindow* window, int button, int action, int mods);
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



//#include "vulkanbase/VulkanBase.h"
//void VulkanBase::initWindow() {
//	glfwInit();
//	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
//	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
//	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
//	glfwSetWindowUserPointer(window, this);
//
//}
