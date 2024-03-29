#include "QueueManager.h"

void QueueManager::Initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) 
{
    QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);

    // Initialize graphics queue
    vkGetDeviceQueue(device, indices.m_GraphicsFamily.value(), 0, &m_GraphicsQueue);

    // Initialize presentation queue
    vkGetDeviceQueue(device, indices.m_PresentFamily.value(), 0, &m_PresentationQueue);
}

QueueFamilyIndices QueueManager::FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.m_GraphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport) {
            indices.m_PresentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}