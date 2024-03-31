#include "Mesh2D.h"
#include <stdexcept>
#include <vulkanbase\VulkanUtil.h>

Mesh2D::Mesh2D(const std::vector<Vertex2D>& vertices, const std::vector<uint16_t>& indices, VkPhysicalDevice physicalDevice, VkDevice device)
    :
    m_Vertices{vertices},
    m_Indices{indices},
    m_VkVertexBuffer{VK_NULL_HANDLE},
    m_VkVertexBufferMemory{VK_NULL_HANDLE},
    m_VkIndexBuffer{VK_NULL_HANDLE},
    m_VkIndexBufferMemory{VK_NULL_HANDLE}
{
    Initialize(physicalDevice, device);
}

void Mesh2D::Initialize(VkPhysicalDevice physicalDevice, VkDevice device)
{
    CreateVertexBuffer(device, physicalDevice);
    CreateIndexBuffer(device, physicalDevice);
}

void Mesh2D::CreateVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice)
{
    VkDeviceSize bufferSize = sizeof(m_Vertices[0]) * m_Vertices.size();
    CreateBuffer(
        device,
        physicalDevice,
        bufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        m_VkVertexBuffer, m_VkVertexBufferMemory);

    void* data;
    vkMapMemory(device, m_VkVertexBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, m_Vertices.data(), (size_t) bufferSize);
    vkUnmapMemory(device, m_VkVertexBufferMemory);
}

void Mesh2D::CreateIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice)
{
    VkDeviceSize bufferSize = sizeof(m_Indices[0]) * m_Indices.size();
    CreateBuffer(
        device,
        physicalDevice,
        bufferSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        m_VkIndexBuffer, m_VkIndexBufferMemory);

    void* data;
    vkMapMemory(device, m_VkIndexBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, m_Indices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, m_VkIndexBufferMemory);
}

void Mesh2D::Draw(VkCommandBuffer buffer)
{
    VkBuffer vertexBuffers[] = { m_VkVertexBuffer };
    VkDeviceSize vertexOffsets[] = { 0 };
    vkCmdBindVertexBuffers(buffer, 0, 1, vertexBuffers, vertexOffsets);

    VkBuffer indexBuffers[] = { m_VkIndexBuffer };
    vkCmdBindIndexBuffer(buffer, m_VkIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

    //vkCmdDraw(buffer, static_cast<uint32_t>(m_Vertices.size()), 1, 0, 0);
    vkCmdDrawIndexed(buffer, static_cast<uint32_t>(m_Indices.size()), 1, 0, 0, 0);

}

void Mesh2D::DestroyMesh(VkDevice device)
{
    vkDestroyBuffer(device, m_VkVertexBuffer, nullptr);
    vkFreeMemory(device, m_VkVertexBufferMemory, nullptr);
    vkDestroyBuffer(device, m_VkIndexBuffer, nullptr);
    vkFreeMemory(device, m_VkIndexBufferMemory, nullptr);
}