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
	createInstance();
	setupDebugMessenger();
	createSurface();

	pickPhysicalDevice();
	createLogicalDevice();

	SwapchainManager::GetInstance().Initialize(instance, m_PhysicalDevice, m_Device, surface, window);

	m_RenderPass = std::make_unique<RenderPass>(m_Device, m_PhysicalDevice);

	SwapchainManager::GetInstance().CreateDepthResources();
	SwapchainManager::GetInstance().CreateFrameBuffers(m_RenderPass->GetHandle());

	m_CommandPool.Initialize(m_Device, QueueManager::GetInstance().FindQueueFamilies(m_PhysicalDevice, surface));
	m_CommandBuffer = m_CommandPool.CreateCommandBuffer();

	BlockMeshGenerator::GetInstance().Init(m_Device, m_PhysicalDevice, m_CommandPool.GetHandle());

	m_pGame = std::make_unique<Game>();
	m_pGame->Init(m_Device, m_PhysicalDevice, m_CommandPool.GetHandle());

	m_BasicGraphicsPipeline2D = std::make_unique<BasicGraphicsPipeline2D>(m_Device, m_RenderPass->GetHandle(), "shaders/shader2D.vert.spv",
		"shaders/shader2D.frag.spv");

	//m_GraphicsPipeline3D = std::make_unique<GraphicsPipeline3D>(m_Device, m_PhysicalDevice, m_RenderPass->GetHandle(), "shaders/shader3D.vert.spv",
	//	"shaders/shader3D.frag.spv", m_pGame->GetTextures());	
	m_LandGraphicsPipeline = std::make_unique<GraphicsPipeline3D>(m_Device, m_PhysicalDevice, m_RenderPass->GetHandle(), "shaders/shaderLand.vert.spv",
		"shaders/shaderLand.frag.spv");

	m_WaterGraphicsPipeline = std::make_unique<GraphicsPipeline3D>(m_Device, m_PhysicalDevice, m_RenderPass->GetHandle(), "shaders/shaderWater.vert.spv",
		"shaders/shaderWater.frag.spv");

	createSyncObjects();
}

void VulkanBase::mainLoop()
{
	float printTimer = 0.f;
	Timer::GetInstance().Start();
	InputManager::GetInstance().Init(window);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		if (InputManager::GetInstance().IsKeyPressed(GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, true);

		m_pGame->Update();
		Render();
	}
	vkDeviceWaitIdle(m_Device);
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

	// 3D
	m_RenderPass->Begin(m_CommandBuffer, SwapchainManager::GetInstance().GetSwapchainFrameBuffers(), imageIndex);

	m_LandGraphicsPipeline->UpdateUniformBuffer(m_Device, imageIndex);
	m_LandGraphicsPipeline->BindPipeline(m_CommandBuffer.GetVkCommandBuffer());
	m_LandGraphicsPipeline->BindDescriptorSets(m_CommandBuffer.GetVkCommandBuffer(), imageIndex);

	m_pGame->RenderLand(m_CommandBuffer.GetVkCommandBuffer(), m_LandGraphicsPipeline->GetPipelineLayout());

	m_WaterGraphicsPipeline->UpdateUniformBuffer(m_Device, imageIndex);
	m_WaterGraphicsPipeline->BindPipeline(m_CommandBuffer.GetVkCommandBuffer());
	m_WaterGraphicsPipeline->BindDescriptorSets(m_CommandBuffer.GetVkCommandBuffer(), imageIndex);

	m_pGame->RenderWater(m_CommandBuffer.GetVkCommandBuffer(), m_WaterGraphicsPipeline->GetPipelineLayout());

	// 2D
	m_BasicGraphicsPipeline2D->BindPipeline(m_CommandBuffer.GetVkCommandBuffer());

	m_pGame->Render2D(m_CommandBuffer.GetVkCommandBuffer());

	m_RenderPass->End(m_CommandBuffer);
}

void VulkanBase::Render()
{
	// multithreading and surface setup (imageIndex).
	vkWaitForFences(m_Device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
	vkResetFences(m_Device, 1, &inFlightFence);

	uint32_t imageIndex;
	auto swapChain = SwapchainManager::GetInstance().GetSwapchain();
	vkAcquireNextImageKHR(m_Device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

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
	vkDestroySemaphore(m_Device, renderFinishedSemaphore, nullptr);
	vkDestroySemaphore(m_Device, imageAvailableSemaphore, nullptr);
	vkDestroyFence(m_Device, inFlightFence, nullptr);

	m_CommandPool.Destroy();

	BlockMeshGenerator::GetInstance().Destroy(m_Device);

	m_BasicGraphicsPipeline2D->DestroyPipeline(m_Device);
	m_LandGraphicsPipeline->DestroyPipeline(m_Device);
	m_WaterGraphicsPipeline->DestroyPipeline(m_Device);

	m_pGame->Destroy(m_Device);
	//m_pGame.reset(nullptr);

	m_RenderPass->Destroy(m_Device);

	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}
	SwapchainManager::GetInstance().Cleanup();

	vkDestroyDevice(m_Device, nullptr);

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
			m_PhysicalDevice = device;
			break;
		}
	}

	if (m_PhysicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

bool VulkanBase::isDeviceSuitable(VkPhysicalDevice device) {
	QueueFamilyIndices indices = QueueManager::GetInstance().FindQueueFamilies(device, surface);
	bool extensionsSupported = checkDeviceExtensionSupport(device);

	// Check for anisotropy filter support, we could change it to a conditional check inside the texture class
	// and then do manually disable it
	// samplerInfo.anisotropyEnable = VK_FALSE;
	// samplerInfo.maxAnisotropy = 1.0f;
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	return indices.isComplete() && extensionsSupported && supportedFeatures.samplerAnisotropy;
}

void VulkanBase::createLogicalDevice() {
	QueueFamilyIndices indices = QueueManager::GetInstance().FindQueueFamilies(m_PhysicalDevice, surface);

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
	deviceFeatures.samplerAnisotropy = VK_TRUE;

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

	if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	QueueManager::GetInstance().Initialize(m_Device, m_PhysicalDevice, surface);
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

	if (vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
		vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
		vkCreateFence(m_Device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {
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