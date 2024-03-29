#pragma once
#include "vulkan/vulkan_core.h"
#include <vector>
#include <optional>

struct QueueFamilyIndices
{
    std::optional<uint32_t> m_GraphicsFamily;
    std::optional<uint32_t> m_PresentFamily;

    bool isComplete() const {
        return m_GraphicsFamily.has_value() && m_PresentFamily.has_value();
    }
};

class QueueManager 
{
public:
    static QueueManager& GetInstance() {
        static QueueManager instance;
        return instance;
    }

    void Initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
    VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
    VkQueue GetPresentationQueue() const { return m_PresentationQueue; }
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

private:
    QueueManager() = default;
    ~QueueManager() = default;

    VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
    VkQueue m_PresentationQueue = VK_NULL_HANDLE;
};