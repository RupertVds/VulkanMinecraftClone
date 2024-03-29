#pragma once
#include <vector>
#include <stdexcept>
#include <optional>
#include <algorithm>
#include <vulkan/vulkan.h>

struct SwapChainSupportDetails 
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct GLFWwindow;

class SwapchainManager final
{
public:
    static SwapchainManager& GetInstance() 
    {
        static SwapchainManager instance;
        return instance;
    }

    void Initialize(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, GLFWwindow* window);
    void Cleanup();

    VkSwapchainKHR GetSwapchain() const { return swapChain; }
    VkExtent2D GetSwapchainExtent() const { return swapChainExtent; }
    VkFormat GetSwapchainImageFormat() const { return swapChainImageFormat; }
    const std::vector<VkImageView>& GetImageViews() const { return swapChainImageViews; }

private:
    SwapchainManager() = default;
    ~SwapchainManager() = default;

    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    VkExtent2D swapChainExtent{};
    VkFormat swapChainImageFormat{};
    std::vector<VkImage> swapChainImages{};
    std::vector<VkImageView> swapChainImageViews{};

    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    GLFWwindow* window = nullptr;

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    void createSwapChain();
    void createImageViews();
};