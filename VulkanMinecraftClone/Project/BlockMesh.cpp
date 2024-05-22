#include "BlockMesh.h"
#include <vulkanbase\VulkanUtil.h>

BlockMesh::BlockMesh(const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices, VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool)
    :
    m_VerticesLand{ vertices },
    m_IndicesLand{ indices },
    m_VkVertexBuffer{ VK_NULL_HANDLE },
    m_VkVertexBufferMemory{ VK_NULL_HANDLE },
    m_VkIndexBuffer{ VK_NULL_HANDLE },
    m_VkIndexBufferMemory{ VK_NULL_HANDLE }
{
    Initialize(physicalDevice, device, commandPool);
}

void BlockMesh::Initialize(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool)
{
    CreateVertexBuffer(device, physicalDevice, commandPool);
    CreateIndexBuffer(device, physicalDevice, commandPool);
}

void BlockMesh::Draw(VkCommandBuffer buffer, VkPipelineLayout pipelineLayout, const glm::vec3& translation) const
{
    VkBuffer vertexBuffers[] = { m_VkVertexBuffer };
    VkDeviceSize vertexOffsets[] = { 0 };
    vkCmdBindVertexBuffers(buffer, 0, 1, vertexBuffers, vertexOffsets);

    VkBuffer indexBuffers[] = { m_VkIndexBuffer };
    vkCmdBindIndexBuffer(buffer, m_VkIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdPushConstants(
        buffer,
        pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT, // Shader stage should match the push constant range in the layout
        0, // Offset within the push constants to update
        sizeof(glm::vec3), // size of the push constants to update
        &translation
    );

    vkCmdDrawIndexed(buffer, static_cast<uint32_t>(m_IndicesLand.size()), 1, 0, 0, 0);
}

void BlockMesh::CreateVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool)
{
    VkDeviceSize bufferSize = sizeof(m_VerticesLand[0]) * m_VerticesLand.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(
        device,
        physicalDevice,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, m_VerticesLand.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    CreateBuffer(
        device,
        physicalDevice,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_VkVertexBuffer, m_VkVertexBufferMemory);

    CopyBuffer(device, commandPool, stagingBuffer, m_VkVertexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void BlockMesh::CreateIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool)
{
    VkDeviceSize bufferSize = sizeof(m_IndicesLand[0]) * m_IndicesLand.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(
        device,
        physicalDevice,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, m_IndicesLand.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    CreateBuffer(
        device,
        physicalDevice,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_VkIndexBuffer, m_VkIndexBufferMemory);

    CopyBuffer(device, commandPool, stagingBuffer, m_VkIndexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void BlockMesh::DestroyMesh(VkDevice device)
{
    vkDestroyBuffer(device, m_VkVertexBuffer, nullptr);
    vkFreeMemory(device, m_VkVertexBufferMemory, nullptr);
    vkDestroyBuffer(device, m_VkIndexBuffer, nullptr);
    vkFreeMemory(device, m_VkIndexBufferMemory, nullptr);
}