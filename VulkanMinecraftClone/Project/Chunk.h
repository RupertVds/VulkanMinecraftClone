#pragma once
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h> // Include Vulkan headers
#include <vulkanbase\VulkanUtil.h>
#include <fstream>
// Json library used: https://github.com/nlohmann/json
#include <vendor/json.hpp>
#include <iostream>
#include "BlockMesh.h"
#include "QueueManager.h"
#include "Timer.h"
#include <mutex>
//#include "vendor/PerlinNoise.hpp"
#include "vendor/SimplexNoise.h"

// IMPORTANT:
// ORDER OF APPEARANCE IN THE JSON FILE MUST MATCH!!!
enum class BlockType : unsigned char
{
    GrassBlock,
    Stone,
    Dirt,
    Sand,
    Log,
    Leaves,
    Water,
    Air
};

// MUST BE SORTED ALFABETICALLY
// BECAUSE THE JSON READER RETURNS IT ALFABETICALLY
enum class Direction : unsigned char
{
    Down,
    East,
    North,
    South,
    Up,
    West
};

struct TextureCoords
{
    unsigned short row;
    unsigned short column;
};

struct BlockData
{
    std::string id;
    std::unordered_map<Direction, TextureCoords> textures;
};

class Chunk
{
public:
    static constexpr int m_Width = 32;
    static constexpr int m_Height = 128;
    static constexpr int m_Depth = 32;
    static constexpr float m_SeaLevel = 0.3f; // Sea level as a fraction of m_Height
    static constexpr float m_MinHeight = 0.0f;
    static constexpr float m_MaxHeight = 1.0f;
public:
    Chunk(const glm::ivec3& position, SimplexNoise* noise, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool);

    void Destroy(VkDevice device)
    {
        // Destroy Vulkan buffers
        vkDestroyBuffer(device, m_VertexBufferLand, nullptr);
        vkFreeMemory(device, m_VertexBufferMemoryLand, nullptr);
        vkDestroyBuffer(device, m_IndexBufferLand, nullptr);
        vkFreeMemory(device, m_IndexBufferMemoryLand, nullptr);
        if (!m_VerticesWater.empty())
        {
            vkDestroyBuffer(device, m_VertexBufferWater, nullptr);
            vkFreeMemory(device, m_VertexBufferMemoryWater, nullptr);
            vkDestroyBuffer(device, m_IndexBufferWater, nullptr);
            vkFreeMemory(device, m_IndexBufferMemoryWater, nullptr);
        }
    }
    void SetBlock(const glm::vec3& position, BlockType blockType)
    {
        if (position.x >= 0 && position.x < m_Width && position.y >= 0 && position.y < m_Height && position.z >= 0 && position.z < m_Depth)
        {
            m_Blocks[GetIndex(position.x, position.y, position.z)] = blockType;
        }
    }

    BlockType GetBlock(const glm::vec3& position) const
    {
        if (position.x >= 0 && position.x < m_Width && position.y >= 0 && position.y < m_Height && position.z >= 0 && position.z < m_Depth)
        {
            return m_Blocks[GetIndex(position.x, position.y, position.z)];
        }
        return BlockType::Air;
    }

    void GenerateMesh();

    void GenerateTerrain();

    bool IsWithinBounds(const glm::ivec3& position) const;

    void RenderLand(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);


    void RenderWater(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);

    void Update();

    const glm::ivec3& GetPosition() const { return m_Position; }

    void SetIsMarkedForDeletion(bool state)
    { 
        m_IsMarkedForDeletion = state; 

        if (state == false)
        {
            m_DeletionTimer = 0;
        }
    }

    bool IsMarkedForDeletion() const { return m_IsMarkedForDeletion; }
    bool IsDeleted() const { return m_IsDeleted; }
private:
    glm::ivec3 m_Position{};
    std::vector<BlockType> m_Blocks;
    std::vector<Vertex> m_VerticesLand;
    std::vector<uint32_t> m_IndicesLand;
    std::vector<Vertex> m_VerticesWater;
    std::vector<uint32_t> m_IndicesWater;
    VkDevice m_Device;

    // Vulkan buffers for land
    VkBuffer m_VertexBufferLand;
    VkDeviceMemory m_VertexBufferMemoryLand;
    VkBuffer m_IndexBufferLand;
    VkDeviceMemory m_IndexBufferMemoryLand;

    // Vulkan buffers for water
    VkBuffer m_VertexBufferWater;
    VkDeviceMemory m_VertexBufferMemoryWater;
    VkBuffer m_IndexBufferWater;
    VkDeviceMemory m_IndexBufferMemoryWater;
    SimplexNoise* m_pNoise{};

    bool m_IsMarkedForDeletion{};
    bool m_IsDeleted{};
    float m_DeletionTimer{};
    float test{};
private:
    size_t GetIndex(int x, int y, int z) const
    {
        return static_cast<size_t>(x) + static_cast<size_t>(y) * m_Width + static_cast<size_t>(z) * m_Width * m_Height;
    }

    bool IsSameBlockType(BlockType blockType, int x, int y, int z) const
    {
        // Check if the neighboring block is out of bounds
        if (x < 0 || x >= m_Width || y < 0 || y >= m_Height || z < 0 || z >= m_Depth)
        {
            // Out of bounds blocks are not the same type
            return false;
        }

        // Get the type of the neighboring block
        BlockType neighborBlockType = GetBlock({ x, y, z });

        // Check if the neighboring block is the same type as the current block
        return (neighborBlockType == blockType);
    }

    bool IsOpaqueBlock(int x, int y, int z) const {
        if (x < 0 || x >= m_Width || y < 0 || y >= m_Height || z < 0 || z >= m_Depth)
        {
            return false; // Out of bounds blocks are considered transparent
        }

        // Get the type of the current block
        BlockType currentBlockType = GetBlock({ x, y, z });

        // Check if the current block is translucent
        if (currentBlockType == BlockType::Air || currentBlockType == BlockType::Leaves || currentBlockType == BlockType::Water)
        {
            return false; // Translucent blocks are considered transparent
        }

        // If none of the neighboring blocks are translucent, consider the current block as opaque
        return true;
    }

    void UpdateVertexBuffer()
    {
        // Update vertex buffer data
        // vkMapMemory(...);
        // memcpy(...);
        // vkUnmapMemory(...);
    }

    void CreateIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool)
    {

    }

    void UpdateIndexBuffer()
    {
        // Update index buffer data
        // vkMapMemory(...);
        // memcpy(...);
        // vkUnmapMemory(...);
    }

    void CreateLandVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool)
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
            m_VertexBufferLand, m_VertexBufferMemoryLand);

        CopyBuffer(device, commandPool, stagingBuffer, m_VertexBufferLand, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }
    void CreateLandIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool)
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
            m_IndexBufferLand, m_IndexBufferMemoryLand);

        CopyBuffer(device, commandPool, stagingBuffer, m_IndexBufferLand, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    void CreateWaterVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool)
    {
        VkDeviceSize bufferSize = sizeof(m_VerticesWater[0]) * m_VerticesWater.size();

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
        memcpy(data, m_VerticesWater.data(), (size_t)bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        CreateBuffer(
            device,
            physicalDevice,
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_VertexBufferWater, m_VertexBufferMemoryWater);

        CopyBuffer(device, commandPool, stagingBuffer, m_VertexBufferWater, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    void CreateWaterIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool)
    {
        VkDeviceSize bufferSize = sizeof(m_IndicesWater[0]) * m_IndicesWater.size();

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
        memcpy(data, m_IndicesWater.data(), (size_t)bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        CreateBuffer(
            device,
            physicalDevice,
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_IndexBufferWater, m_IndexBufferMemoryWater);

        CopyBuffer(device, commandPool, stagingBuffer, m_IndexBufferWater, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }
    void AddFaceVertices(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, BlockType blockType, Direction direction, const glm::vec3& position);
    //void AddFaceVertices(BlockType blockType, Direction direction, const glm::vec3& position);
};