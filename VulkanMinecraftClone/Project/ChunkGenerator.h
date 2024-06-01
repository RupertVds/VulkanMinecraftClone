#pragma once
#include "Chunk.h"
#include <vector>
#include <glm/glm.hpp>
#include <Camera.h>

#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>
#include "CommandPool.h"

namespace std 
{
    template <>
    struct hash<glm::ivec3>
    {
        size_t operator()(const glm::ivec3& v) const
        {
            // A simple hash function for glm::ivec3
            // You can use any hash function suitable for your use case
            return std::hash<int>()(v.x) ^ std::hash<int>()(v.y) ^ std::hash<int>()(v.z);
        }
    };
}

class SimplexNoise;

class ChunkGenerator final
{
private:
    static const int m_ViewDistance;
    static const int m_LoadDistance;
    static const int m_Padding; 
    static const int m_NoiseFractals; 
    static const float m_ChunkDeletionTime; 
    std::unique_ptr<SimplexNoise> m_pSimplexNoise;
    std::unordered_map<BlockType, BlockData> m_BlockData{};
    float m_WaterTimer{};

    struct Offset
    {
        int x;
        int y;
        int z;
    };

    std::unordered_map<Direction, Offset> m_FaceOffsets{
    {Direction::Down, {0, -1, 0}},
    {Direction::East, {1, 0, 0}},
    {Direction::North, {0, 0, -1}},
    {Direction::South, {0, 0, 1}},
    {Direction::Up, {0, 1, 0}},
    {Direction::West, {-1, 0, 0}}
    };
    //siv::PerlinNoise::seed_type m_Seed{ 0 };
public:
    static ChunkGenerator& GetInstance()
    {
        static ChunkGenerator instance;
        return instance;
    }
public:
    void Init(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool);

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
            auto texturesJson = blocks[i]["textures"];
            int index = 0; // Counter for direction index
            for (auto it = texturesJson.begin(); it != texturesJson.end(); ++it) {
                Direction direction = static_cast<Direction>(index);
                blockData.textures[direction] = { it.value()["row"], it.value()["col"] };
                ++index; // Increment index for next direction
            }

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

    const std::unordered_map<BlockType, BlockData>& GetBlockData() const
    {
        return m_BlockData;
    }

    const std::unordered_map<Direction, Offset>& GetFaceOffsets() const
    {
        return m_FaceOffsets;
    }

    float GetWaterTimer() const { return m_WaterTimer; }

    void RenderLand(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
    {
        for (const auto& chunk : m_ChunkMap)
        {
            if (!chunk.second->IsMarkedForDeletion())
            {
                chunk.second->RenderLand(commandBuffer, pipelineLayout);
            }
        }
    }

    void RenderWater(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
    {
        // Retrieve the camera position
        glm::vec3 cameraPosition = Camera::GetInstance().m_Position;

        // Vector to store chunks with their distances
        std::vector<std::pair<float, Chunk*>> chunkDistances;

        // Calculate the distance from the camera to the center of each chunk
        for (const auto& chunk : m_ChunkMap)
        {
            if (!chunk.second->IsMarkedForDeletion())
            {
                glm::vec3 chunkPosition = glm::vec3(chunk.second->GetPosition());
                glm::vec3 chunkCenter = chunkPosition + glm::vec3(Chunk::m_Width / 2.0f, Chunk::m_Height / 2.0f, Chunk::m_Depth / 2.0f);
                float distance = glm::distance(cameraPosition, chunkCenter);
                chunkDistances.emplace_back(distance, chunk.second.get());
            }
        }

        // Sort chunks by distance from the camera in ascending order
        std::sort(chunkDistances.begin(), chunkDistances.end());

        // Render the water in the sorted order
        for (const auto& [distance, chunk] : chunkDistances)
        {
            chunk->RenderWater(commandBuffer, pipelineLayout);
        }
    }

    void Update()
    {
        // Check if the player has moved to a new chunk
        glm::ivec3 newPlayerChunkPosition = CalculateChunkPosition(Camera::GetInstance().m_Position);
        if (newPlayerChunkPosition != m_PlayerChunkPosition)
        {
            // Update the player's chunk position
            m_PlayerChunkPosition = newPlayerChunkPosition;

            // Update chunks around the player
            UpdateChunksAroundPlayer();
        }

        m_WaterTimer += Timer::GetInstance().GetElapsed();;

        for (auto& chunk : m_ChunkMap)
        {
            chunk.second->Update();
        }

        // Destroy all chunks that are to be destroyed if any
        DestroyDeletedChunks();
    }

    int GetHeight(const glm::ivec3& globalPosition)
    {
        float noise = m_pSimplexNoise->fractal(m_NoiseFractals, globalPosition.x, globalPosition.z);
        int terrainHeight = static_cast<int>(noise * (Chunk::m_Height * (Chunk::m_MaxHeight - Chunk::m_MinHeight)) + Chunk::m_Height * Chunk::m_MinHeight);

        // Clamp terrainHeight to ensure it's within the range [0, m_Height]
        terrainHeight = std::clamp(terrainHeight, 0, Chunk::m_Height);

        return terrainHeight;
    }

    void DestroyDeletedChunks()
    {
        auto it = m_ChunkMap.begin();
        while (it != m_ChunkMap.end())
        {
            if ((*it).second->IsDeleted())
            {
                (*it).second->Destroy(m_Device);
                it = m_ChunkMap.erase(it); // Erase the current element and get the iterator to the next element
                std::cout << "Destroyed a chunk!\n";
            }
            else
            {
                ++it; // Move to the next element
            }
        }
    }

    void Destroy()
    {
        for (auto& chunk : m_ChunkMap)
        {
            chunk.second->Destroy(m_Device);
        }
    }

    float GetChunkDeletionTime() const { return m_ChunkDeletionTime; }

    Chunk* GetChunkAtPosition(const glm::ivec3& position)
    {
        auto it = m_ChunkMap.find(position);
        if (it != m_ChunkMap.end())
        {
            return it->second.get();
        }
        return nullptr; // or handle the case where the chunk is not found
    }

private:
    //std::vector<std::unique_ptr<Chunk>> m_Chunks;
    std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>> m_ChunkMap;
    VkDevice m_Device;
    VkPhysicalDevice m_PhysicalDevice;
    VkCommandPool m_CommandPool;

    glm::ivec3 m_PlayerChunkPosition;

    glm::ivec3 CalculateChunkPosition(const glm::vec3& position) const
    {
        // Calculate the chunk position based on the player's position
        return {
            static_cast<int>(position.x) / Chunk::m_Width,
            0, // Fixed height of 0
            static_cast<int>(position.z) / Chunk::m_Depth
        };
    }

    void UpdateChunksAroundPlayer()
    {
        // Calculate the range of chunks to be loaded around the player
        int minX = m_PlayerChunkPosition.x - m_LoadDistance;
        int maxX = m_PlayerChunkPosition.x + m_LoadDistance;
        int minZ = m_PlayerChunkPosition.z - m_LoadDistance;
        int maxZ = m_PlayerChunkPosition.z + m_LoadDistance;

        // Mark chunks for deletion outside the view distance
        for (auto& chunk : m_ChunkMap)
        {
            glm::ivec3 chunkPosition = CalculateChunkPosition(chunk.second->GetPosition());
            bool outsideViewDistance = chunkPosition.x < m_PlayerChunkPosition.x - m_ViewDistance ||
                chunkPosition.x > m_PlayerChunkPosition.x + m_ViewDistance ||
                chunkPosition.z < m_PlayerChunkPosition.z - m_ViewDistance ||
                chunkPosition.z > m_PlayerChunkPosition.z + m_ViewDistance;
            if (outsideViewDistance)
            {
                chunk.second->SetIsMarkedForDeletion(true);
            }
            else
            {
                chunk.second->SetIsMarkedForDeletion(false);
            }
        }

        // Create chunks inside the load distance if not already existing
        for (int x = minX; x <= maxX; ++x)
        {
            for (int z = minZ; z <= maxZ; ++z)
            {
                glm::ivec3 chunkPosition = { x, 0, z };
                if (!IsChunkLoaded(chunkPosition))
                {
                    // Create and load the chunk
                    //m_Chunks.emplace_back(std::make_unique<Chunk>(
                    //    glm::ivec3(
                    //        chunkPosition.x * Chunk::m_Width,
                    //        chunkPosition.y * Chunk::m_Height,
                    //        chunkPosition.z * Chunk::m_Depth),
                    //    //m_Seed,
                    //    m_pSimplexNoise.get(),
                    //    m_Device,
                    //    m_PhysicalDevice,
                    //    m_CommandPool));
                    m_ChunkMap[chunkPosition] = std::make_unique<Chunk>(
                        glm::ivec3(
                            chunkPosition.x * Chunk::m_Width,
                            chunkPosition.y * Chunk::m_Height,
                            chunkPosition.z * Chunk::m_Depth),
                        //m_Seed,
                        m_pSimplexNoise.get(),
                        m_Device,
                        m_PhysicalDevice,
                        m_CommandPool);

                    // Load neighbor chunks based on padding
                    LoadNeighborChunks(chunkPosition);
                }
            }
        }
    }

    void LoadNeighborChunks(const glm::ivec3& chunkPosition)
    {
        for (int dx = -m_Padding; dx <= m_Padding; ++dx)
        {
            for (int dz = -m_Padding; dz <= m_Padding; ++dz)
            {
                if (dx == 0 && dz == 0) continue; // Skip the center chunk (already loaded)
                glm::ivec3 neighborChunkPosition = { chunkPosition.x + dx, 0, chunkPosition.z + dz };
                if (!IsChunkLoaded(neighborChunkPosition))
                {
                    m_ChunkMap[neighborChunkPosition] = std::make_unique<Chunk>(
                        glm::ivec3(
                            neighborChunkPosition.x * Chunk::m_Width,
                            neighborChunkPosition.y * Chunk::m_Height,
                            neighborChunkPosition.z * Chunk::m_Depth),
                        //m_Seed,
                        m_pSimplexNoise.get(),
                        m_Device,
                        m_PhysicalDevice,
                        m_CommandPool);
                }
            }
        }
    }

    bool IsChunkLoaded(const glm::ivec3& chunkPosition) const
    {
        // Check if a chunk at the given position is already loaded
        return m_ChunkMap.find(chunkPosition) != m_ChunkMap.end();
    }

private:
    ChunkGenerator() = default;
};
