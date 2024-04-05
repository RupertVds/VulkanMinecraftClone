#include "vulkanbase/VulkanBase.h"

void VulkanBase::initWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(WIDTH, HEIGHT, "Rupert Vanderstappen", nullptr, nullptr);
}

void VulkanBase::initVulkan()
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
	m_RenderPass = std::make_unique<RenderPass>(device);
	//m_RenderPass3D = std::make_unique<RenderPass>(device);

	m_BasicGraphicsPipeline2D = std::make_unique<BasicGraphicsPipeline2D>(device, m_RenderPass->GetHandle(), "shaders/shader2D.vert.spv",
		"shaders/shader2D.frag.spv");

	m_GraphicsPipeline3D = std::make_unique<GraphicsPipeline3D>(device, physicalDevice, m_RenderPass->GetHandle(), "shaders/shader3D.vert.spv",
		"shaders/shader3D.frag.spv");

	//createFrameBuffers();
	SwapchainManager::GetInstance().CreateFrameBuffers(m_RenderPass->GetHandle());

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

void VulkanBase::mainLoop()
{
	float printTimer = 0.f;
	Timer::GetInstance().Start();
	InputManager::GetInstance().Init(window);
	Camera::GetInstance().Init(&InputManager::GetInstance(), { 0, 0, 5 });

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

		if (InputManager::GetInstance().IsKeyPressed(GLFW_KEY_ESCAPE))
		{
			glfwSetWindowShouldClose(window, true);
		}

		// week 06
		drawFrame();
	}
	vkDeviceWaitIdle(device);
	Timer::GetInstance().Stop();
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

	//TODO: Change to Game->Render(VkCommandBuffer, uint32_t);
	m_RenderPass->Begin(m_CommandBuffer, SwapchainManager::GetInstance().GetSwapchainFrameBuffers(), imageIndex);

	m_BasicGraphicsPipeline2D->BindPipeline(m_CommandBuffer.GetVkCommandBuffer());

	m_Scene2D->Render(m_CommandBuffer.GetVkCommandBuffer());

	m_GraphicsPipeline3D->BindPipeline(m_CommandBuffer.GetVkCommandBuffer());
	m_GraphicsPipeline3D->BindDescriptorSets(m_CommandBuffer.GetVkCommandBuffer(), imageIndex);

	m_Scene3D->Render(m_CommandBuffer.GetVkCommandBuffer(), m_GraphicsPipeline3D->GetPipelineLayout());

	m_RenderPass->End(m_CommandBuffer);
}

void VulkanBase::drawFrame()
{
	// multithreading and surface setup (imageIndex).
	vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
	vkResetFences(device, 1, &inFlightFence);

	uint32_t imageIndex;
	auto swapChain = SwapchainManager::GetInstance().GetSwapchain();
	vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	m_GraphicsPipeline3D->UpdateUniformBuffer(device, imageIndex);

	// Combine this to record buffer?
	m_CommandBuffer.Reset();
	m_CommandBuffer.BeginRecording();
	drawFrame(imageIndex);
	m_CommandBuffer.EndRecording();

	// the commandbuffer has to be sent to the gpu, otherwise you see nothing.
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	m_CommandBuffer.Submit(submitInfo);

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(QueueManager::GetInstance().GetGraphicsQueue(), 1, &submitInfo, inFlightFence) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	vkQueuePresentKHR(QueueManager::GetInstance().GetPresentationQueue(), &presentInfo);
}

void VulkanBase::cleanup()
{
	vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
	vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
	vkDestroyFence(device, inFlightFence, nullptr);

	m_CommandPool.Destroy();

	m_BasicGraphicsPipeline2D->DestroyPipeline(device);
	m_GraphicsPipeline3D->DestroyPipeline(device);
	m_Scene2D->CleanUp();
	m_Scene3D->CleanUp();

	m_RenderPass->Destroy(device);

	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}
	SwapchainManager::GetInstance().Cleanup();

	vkDestroyDevice(device, nullptr);

	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);

	glfwDestroyWindow(window);
	glfwTerminate();
}

void VulkanBase::createSurface()
{
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}
}

void VulkanBase::pickPhysicalDevice() {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices{ deviceCount };
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	for (const auto& device : devices) {
		if (isDeviceSuitable(device)) {
			physicalDevice = device;
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

bool VulkanBase::isDeviceSuitable(VkPhysicalDevice device) {
	QueueFamilyIndices indices = QueueManager::GetInstance().FindQueueFamilies(device, surface);
	bool extensionsSupported = checkDeviceExtensionSupport(device);


	return indices.isComplete() && extensionsSupported;
}

void VulkanBase::createLogicalDevice() {
	QueueFamilyIndices indices = QueueManager::GetInstance().FindQueueFamilies(physicalDevice, surface);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.m_GraphicsFamily.value(), indices.m_PresentFamily.value() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkDeviceQueueCreateInfo queueCreateInfo{};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices.m_GraphicsFamily.value();
	queueCreateInfo.queueCount = 1;

	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	QueueManager::GetInstance().Initialize(device, physicalDevice, surface);
}

bool checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

void VulkanBase::createInstance()
{
	if (enableValidationLayers && !checkValidationLayerSupport())
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "MinecraftClone";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;

		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create instance!");
	}
}

void VulkanBase::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

void VulkanBase::setupDebugMessenger()
{
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

void VulkanBase::createSyncObjects()
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
		vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
		vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {
		throw std::runtime_error("failed to create synchronization objects for a frame!");
	}

}

std::vector<const char*> VulkanBase::getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

bool VulkanBase::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}