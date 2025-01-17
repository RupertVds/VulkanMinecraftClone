#pragma once
#include <vector>
#include <stdexcept>
#include <optional>
#include <algorithm>
#include <vulkan/vulkan.h>

struct SwapChainSupportDetails 
{
    VkSurfaceCapabilitiesKHR m_Capabilities;
    std::vector<VkSurfaceFormatKHR> m_Formats;
    std::vector<VkPresentModeKHR> m_PresentModes;
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

    VkSwapchainKHR GetSwapchain() const { return m_SwapChain; }
    VkExtent2D GetSwapchainExtent() const { return m_SwapChainExtent; }
    VkFormat GetSwapchainImageFormat() const { return m_SwapChainImageFormat; }
    const std::vector<VkImageView>& GetImageViews() const { return m_SwapChainImageViews; }
    void CreateFrameBuffers(VkRenderPass renderPass);
    void CreateDepthResources();
    const std::vector<VkFramebuffer>& GetSwapchainFrameBuffers() const { return m_SwapChainFramebuffers; }
private:
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
    SwapchainManager() = default;
    ~SwapchainManager() = default;

    VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
    VkExtent2D m_SwapChainExtent{};
    VkFormat m_SwapChainImageFormat{};
    std::vector<VkImage> m_SwapChainImages{};
    std::vector<VkImageView> m_SwapChainImageViews{};
    std::vector<VkFramebuffer> m_SwapChainFramebuffers{};

    VkImage m_DepthImage{};
    VkImageView m_DepthImageView{};
    VkDeviceMemory m_DepthImageMemory{};

    // TODO: remove these
    VkInstance m_Instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkDevice m_Device = VK_NULL_HANDLE;
    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
    GLFWwindow* m_Window = nullptr;

    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    void CreateSwapChain();
    void CreateImageViews();
    VkFormat FindDepthFormat();

};