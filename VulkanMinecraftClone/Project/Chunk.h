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
#include <BlockMesh.h>

// IMPORTANT:
// ORDER OF APPEARANCE IN THE JSON FILE MUST MATCH!!!
enum class BlockType : unsigned short
{
    GrassTop,
    Stone,
    Dirt,
    GrassSide,
    Sand,
    LogSide,
    LogTop,
    Leaves,
    Water,
    Air
};

struct BlockData
{
    std::string id;
    int row;
    int column;
};

enum class Direction : unsigned short
{
    Up,
    Down,
    North,
    East,
    South,
    West
};


class Chunk
{
public:
    Chunk(const glm::vec3& position, int width, int height, int depth, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool)
        :
        m_Position{ position },
        m_Width(width), m_Height(height), m_Depth(depth)
    {
        m_Blocks.resize(m_Width * m_Height * m_Depth, BlockType::Air);

        LoadBlockData("textures/blockdata.json");
        GenerateMesh();
        // Create Vulkan buffers
        CreateVertexBuffer(device, physicalDevice, commandPool);
        CreateIndexBuffer(device, physicalDevice, commandPool);
    }

    void Destroy(VkDevice device)
    {
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

    void GenerateMesh()
    {
        // Generate mesh data for the chunk
        // Implementation depends on your mesh generation algorithm
        m_Vertices.clear();
        m_Indices.clear();

        for (int x = 0; x < m_Width; ++x) {
            for (int y = 0; y < m_Height; ++y) {
                for (int z = 0; z < m_Depth; ++z) {
                    //BlockType blockType = GetBlock({ x, y, z });
                    BlockType blockType;

                    // Fill the chunk with grass on top
                    if (y == m_Height - 1) {
                        blockType = BlockType::GrassTop;
                    }
                    // Fill the chunk with dirt below the grass
                    else if (y >= m_Height / 2) {
                        blockType = BlockType::Dirt;
                    }
                    // Fill the rest of the chunk with stone
                    else {
                        blockType = BlockType::Stone;
                    }


                    // Skip air blocks
                    if (blockType == BlockType::Air) {
                        continue;
                    }

                    // Check each face of the block
                    for (const auto& [direction, offset] : m_FaceOffsets)
                    {
                        int nx = x + offset.x;
                        int ny = y + offset.y;
                        int nz = z + offset.z;

                        // If the adjacent block is air or out of bounds, add face vertices
                        if (!IsOpaqueBlock(nx, ny, nz)) {
                            AddFaceVertices(blockType, direction, glm::vec3(x, y, z));
                        }
                    }
                }
            }
        }

        // Update Vulkan buffers
        UpdateVertexBuffer();
        UpdateIndexBuffer();
    }

    void Render(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const
    {
        // Bind vertex buffer
        VkBuffer vertexBuffers[] = { m_VkVertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        // Bind index buffer
        vkCmdBindIndexBuffer(commandBuffer, m_VkIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

        // Draw indexed
        vkCmdPushConstants(
            commandBuffer,
            pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT, // Shader stage should match the push constant range in the layout
            0, // Offset within the push constants to update
            sizeof(glm::vec3), // size of the push constants to update
            &m_Position
        );

        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_Indices.size()), 1, 0, 0, 0);
    }

private:
    glm::vec3 m_Position{};
    int m_Width;
    int m_Height;
    int m_Depth;
    std::vector<BlockType> m_Blocks;
    std::vector<Vertex> m_Vertices;
    std::vector<uint16_t> m_Indices;
    VkDevice m_Device;
    VkBuffer m_VkVertexBuffer;
    VkDeviceMemory m_VkVertexBufferMemory;
    VkBuffer m_VkIndexBuffer;
    VkDeviceMemory m_VkIndexBufferMemory;

    std::unordered_map<BlockType, BlockData> m_BlockData{};

    struct Offset
    {
        int x;
        int y;
        int z;
    };

    std::unordered_map<Direction, Offset> m_FaceOffsets{
    {Direction::Up, {0, 1, 0}},
    {Direction::Down, {0, -1, 0}},
    {Direction::North, {0, 0, -1}},
    {Direction::East, {1, 0, 0}},
    {Direction::South, {0, 0, 1}},
    {Direction::West, {-1, 0, 0}}
    };
private:
    size_t GetIndex(int x, int y, int z) const
    {
        return static_cast<size_t>(x) + static_cast<size_t>(y) * m_Width + static_cast<size_t>(z) * m_Width * m_Height;
    }

    // Check if a block at given position is opaque
    bool IsOpaqueBlock(int x, int y, int z) const {
        if (x < 0 || x >= m_Width || y < 0 || y >= m_Height || z < 0 || z >= m_Depth)
        {
            return false; // Out of bounds blocks are considered transparent
        }
        return GetBlock({ x, y, z }) != BlockType::Air; // Check if the block is opaque
    }

    void CreateVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool)
    {
        VkDeviceSize bufferSize = sizeof(m_Vertices[0]) * m_Vertices.size();

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
        memcpy(data, m_Vertices.data(), (size_t)bufferSize);
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
        VkDeviceSize bufferSize = sizeof(m_Indices[0]) * m_Indices.size();

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
        memcpy(data, m_Indices.data(), (size_t)bufferSize);
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

    void LoadBlockData(const std::string& jsonFilePath)
    {
        // Open the JSON file
        std::ifstream jsonFile(jsonFilePath);
        if (!jsonFile.is_open()) {
            // Handle error: unable to open JSON file
            return;
        }

        // Parse JSON data
        nlohmann::json jsonData;
        jsonFile >> jsonData;

        // Check if "blocks" array exists
        if (!jsonData.contains("blocks")) {
            // Handle error: "blocks" array not found
            return;
        }

        // Get the array of blocks
        auto blocks = jsonData["blocks"];

        // Iterate over each block type
        for (size_t i = 0; i < blocks.size(); ++i) {
            // Extract block data
            BlockData blockData;
            blockData.id = blocks[i]["id"];
            blockData.row = blocks[i]["row"];
            blockData.column = blocks[i]["col"];

            // Add block data to the map
            m_BlockData[GetBlockType(i)] = blockData;
        }
    }

    BlockType GetBlockType(size_t index) const
    {
        // Return the block type based on the index
        // We assume that the block types are defined in the same order as in the JSON file
        return static_cast<BlockType>(index);
    }


    void AddFaceVertices(BlockType blockType, Direction direction, const glm::vec3& position)
    {
        // Check if block data exists for the given block type
        auto it = m_BlockData.find(blockType);
        if (it == m_BlockData.end()) {
            // Handle error: Block data not found for the given block type
            std::cout << "ERROR: BLOCK DATA NOT FOUND FOR THE GIVEN BLOCK TYPE!\n";
            return;
        }

        const BlockData& blockData = it->second;

        constexpr float textureAtlasWidth = 16;
        constexpr float textureAtlasHeight = 16;

        // Calculate texture coordinates based on block data
        float texCoordLeft = blockData.column * (1.0f / textureAtlasWidth);
        float texCoordRight = (blockData.column + 1) * (1.0f / textureAtlasWidth);
        float texCoordTop = blockData.row * (1.0f / textureAtlasHeight);
        float texCoordBottom = (blockData.row + 1) * (1.0f / textureAtlasHeight);

        // Calculate the base position of the face
        glm::vec3 basePosition = position;

        // Adjust base position according to face direction
        switch (direction) {
        case Direction::Up:
            basePosition.y += 0.5f;
            break;
        case Direction::Down:
            basePosition.y -= 0.5f;
            break;
        case Direction::North:
            basePosition.z -= 0.5f;
            break;
        case Direction::East:
            basePosition.x += 0.5f;
            break;
        case Direction::South:
            basePosition.z += 0.5f;
            break;
        case Direction::West:
            basePosition.x -= 0.5f;
            break;
        }

        // Calculate normals
        glm::vec3 normal;
        switch (direction) {
        case Direction::Up:
            normal = { 0.0f, 1.0f, 0.0f };
            break;
        case Direction::Down:
            normal = { 0.0f, -1.0f, 0.0f };
            break;
        case Direction::North:
            normal = { 0.0f, 0.0f, -1.0f };
            break;
        case Direction::East:
            normal = { 1.0f, 0.0f, 0.0f };
            break;
        case Direction::South:
            normal = { 0.0f, 0.0f, 1.0f };
            break;
        case Direction::West:
            normal = { -1.0f, 0.0f, 0.0f };
            break;
        }

        // Add vertices for the face
        // Define vertices for the face
        std::vector<Vertex> faceVertices;
        faceVertices.reserve(4); // We need 4 vertices for a quad

        // Add vertices for the face based on the direction
        switch (direction) {
        case Direction::Up:
            faceVertices.push_back({ { basePosition.x - 0.5f, basePosition.y, basePosition.z + 0.5f }, normal, { texCoordLeft, texCoordBottom } });
            faceVertices.push_back({ { basePosition.x + 0.5f, basePosition.y, basePosition.z + 0.5f }, normal, { texCoordRight, texCoordBottom } });
            faceVertices.push_back({ { basePosition.x + 0.5f, basePosition.y, basePosition.z - 0.5f }, normal, { texCoordRight, texCoordTop } });
            faceVertices.push_back({ { basePosition.x - 0.5f, basePosition.y, basePosition.z - 0.5f }, normal, { texCoordLeft, texCoordTop } });
            break;

        case Direction::Down:
            faceVertices.push_back({ { basePosition.x - 0.5f, basePosition.y, basePosition.z - 0.5f }, normal, { texCoordLeft, texCoordTop } });
            faceVertices.push_back({ { basePosition.x + 0.5f, basePosition.y, basePosition.z - 0.5f }, normal, { texCoordRight, texCoordTop } });
            faceVertices.push_back({ { basePosition.x + 0.5f, basePosition.y, basePosition.z + 0.5f }, normal, { texCoordRight, texCoordBottom } });
            faceVertices.push_back({ { basePosition.x - 0.5f, basePosition.y, basePosition.z + 0.5f }, normal, { texCoordLeft, texCoordBottom } });
            break;

        case Direction::North:
            faceVertices.push_back({ { basePosition.x - 0.5f, basePosition.y - 0.5f, basePosition.z }, normal, { texCoordLeft, texCoordBottom } });
            faceVertices.push_back({ { basePosition.x - 0.5f, basePosition.y + 0.5f, basePosition.z }, normal, { texCoordLeft, texCoordTop } });
            faceVertices.push_back({ { basePosition.x + 0.5f, basePosition.y + 0.5f, basePosition.z }, normal, { texCoordRight, texCoordTop } });
            faceVertices.push_back({ { basePosition.x + 0.5f, basePosition.y - 0.5f, basePosition.z }, normal, { texCoordRight, texCoordBottom } });
            break;

        case Direction::South:
            faceVertices.push_back({ { basePosition.x + 0.5f, basePosition.y - 0.5f, basePosition.z }, normal, { texCoordRight, texCoordBottom } });
            faceVertices.push_back({ { basePosition.x + 0.5f, basePosition.y + 0.5f, basePosition.z }, normal, { texCoordRight, texCoordTop } });
            faceVertices.push_back({ { basePosition.x - 0.5f, basePosition.y + 0.5f, basePosition.z }, normal, { texCoordLeft, texCoordTop } });
            faceVertices.push_back({ { basePosition.x - 0.5f, basePosition.y - 0.5f, basePosition.z }, normal, { texCoordLeft, texCoordBottom } });
            break;

        case Direction::East:
            faceVertices.push_back({ { basePosition.x, basePosition.y - 0.5f, basePosition.z + 0.5f }, normal, { texCoordLeft, texCoordBottom } });
            faceVertices.push_back({ { basePosition.x, basePosition.y - 0.5f, basePosition.z - 0.5f }, normal, { texCoordRight, texCoordBottom } });
            faceVertices.push_back({ { basePosition.x, basePosition.y + 0.5f, basePosition.z - 0.5f }, normal, { texCoordRight, texCoordTop } });
            faceVertices.push_back({ { basePosition.x, basePosition.y + 0.5f, basePosition.z + 0.5f }, normal, { texCoordLeft, texCoordTop } });
            break;

        case Direction::West:
            faceVertices.push_back({ { basePosition.x, basePosition.y - 0.5f, basePosition.z - 0.5f }, normal, { texCoordLeft, texCoordBottom } });
            faceVertices.push_back({ { basePosition.x, basePosition.y - 0.5f, basePosition.z + 0.5f }, normal, { texCoordRight, texCoordBottom } });
            faceVertices.push_back({ { basePosition.x, basePosition.y + 0.5f, basePosition.z + 0.5f }, normal, { texCoordRight, texCoordTop } });
            faceVertices.push_back({ { basePosition.x, basePosition.y + 0.5f, basePosition.z - 0.5f }, normal, { texCoordLeft, texCoordTop } });
            break;
        }


        // Add vertices to the m_Vertices vector
        size_t vertexOffset = m_Vertices.size();
        for (const Vertex& vertex : faceVertices)
        {
            m_Vertices.emplace_back(vertex);
        }

        // Add indices to the m_Indices vector
        m_Indices.emplace_back(vertexOffset);
        m_Indices.emplace_back(vertexOffset + 1);
        m_Indices.emplace_back(vertexOffset + 2);
        m_Indices.emplace_back(vertexOffset + 2);
        m_Indices.emplace_back(vertexOffset + 3);
        m_Indices.emplace_back(vertexOffset);
    }
};