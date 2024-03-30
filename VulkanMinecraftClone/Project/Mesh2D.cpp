#include "Mesh2D.h"
#include <stdexcept>

Mesh2D::Mesh2D(const std::vector<Vertex2D>& vertices, VkPhysicalDevice physicalDevice, VkDevice device)
    :
    m_Vertices{vertices},
    m_VkBuffer{VK_NULL_HANDLE},
    m_VkBufferMemory{VK_NULL_HANDLE}
{
    Initialize(physicalDevice, device);
}

void Mesh2D::Initialize(VkPhysicalDevice physicalDevice, VkDevice device)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(m_Vertices[0]) * m_Vertices.size();
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &m_VkBuffer) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create vertex buffer!");
    }

    VkMemoryRequirements memRequirements{};
    vkGetBufferMemoryRequirements(device, m_VkBuffer, &memRequirements);


    VkPhysicalDeviceMemoryProperties memProperties{};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(physicalDevice, memProperties,
        memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    if (vkAllocateMemory(device, &allocInfo, nullptr, &m_VkBufferMemory) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }

    vkBindBufferMemory(device, m_VkBuffer, m_VkBufferMemory, 0);

    void* data;
    vkMapMemory(device, m_VkBufferMemory, 0, bufferInfo.size, 0, &data);
    memcpy(data, m_Vertices.data(), (size_t)bufferInfo.size);
    vkUnmapMemory(device, m_VkBufferMemory);
}

void Mesh2D::DestroyMesh(VkDevice device)
{
    vkDestroyBuffer(device, m_VkBuffer, nullptr);
    vkFreeMemory(device, m_VkBufferMemory, nullptr);
}

void Mesh2D::Draw(VkCommandBuffer buffer)
{
    VkBuffer vertexBuffers[] = { m_VkBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(buffer, 0, 1, vertexBuffers, offsets);

    vkCmdDraw(buffer, static_cast<uint32_t>(m_Vertices.size()), 1, 0, 0);
}

uint32_t Mesh2D::FindMemoryType(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties memProperties, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}
