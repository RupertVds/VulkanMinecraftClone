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
    static constexpr float m_SeaLevel = 0.1f; // Sea level as a fraction of m_Height
    static constexpr float m_MinHeight = 0.0f;
    static constexpr float m_MaxHeight = 1.0f;
public:
    Chunk(const glm::ivec3& position, SimplexNoise* noise, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool);

    void Destroy(VkDevice device)
    {
        // Destroy Vulkan buffers
        vkDestroyBuffer(device, m_VkVertexBuffer, nullptr);
        vkFreeMemory(device, m_VkVertexBufferMemory, nullptr);
        vkDestroyBuffer(device, m_VkIndexBuffer, nullptr);
        vkFreeMemory(device, m_VkIndexBufferMemory, nullptr);
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

    void Render(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
    {
        // Bind vertex buffer
        VkBuffer vertexBuffers[] = { m_VkVertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        // Bind index buffer
        vkCmdBindIndexBuffer(commandBuffer, m_VkIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

        // Draw indexed
        vkCmdPushConstants(
            commandBuffer,
            pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT, // Shader stage should match the push constant range in the layout
            0, // Offset within the push constants to update
            sizeof(glm::ivec3), // size of the push constants to update
            &m_Position
        );

        // Draw triangles directly from the vertex buffer
        //vkCmdDraw(commandBuffer, static_cast<uint32_t>(m_Vertices.size()), 1, 0, 0);

        // Submit rendering commands
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_IndicesLand.size()), 1, 0, 0, 0);
    }

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
    VkBuffer m_VkVertexBuffer;
    VkDeviceMemory m_VkVertexBufferMemory;
    VkBuffer m_VkIndexBuffer;
    VkDeviceMemory m_VkIndexBufferMemory;
    SimplexNoise* m_pNoise{};

    bool m_IsMarkedForDeletion{};
    bool m_IsDeleted{};
    float m_DeletionTimer{};
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

    void CreateVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool)
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

    void UpdateVertexBuffer()
    {
        // Update vertex buffer data
        // vkMapMemory(...);
        // memcpy(...);
        // vkUnmapMemory(...);
    }

    void CreateIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool)
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

    void UpdateIndexBuffer()
    {
        // Update index buffer data
        // vkMapMemory(...);
        // memcpy(...);
        // vkUnmapMemory(...);
    }

    void AddFaceVertices(BlockType blockType, Direction direction, const glm::vec3& position);
};