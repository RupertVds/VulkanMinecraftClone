#pragma once
#include "Chunk.h"
#include <vector>
#include <glm/glm.hpp>
#include <Camera.h>

#include <vector>
#include <algorithm>

class ChunkGenerator final
{
public:
    static ChunkGenerator& GetInstance()
    {
        static ChunkGenerator instance;
        return instance;
    }

public:
    void Init(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool)
    {
        this->m_Device = device;
        this->m_PhysicalDevice = physicalDevice;
        this->m_CommandPool = commandPool;

        // Initialize the player's chunk position
        m_PlayerChunkPosition = { 0, 0, 0 };

        // Initialize chunks around the player
        UpdateChunksAroundPlayer();
    }

    void Render(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
    {
        for (const auto& chunk : m_Chunks)
        {
            if (!chunk->IsMarkedForDeletion())
            {
                chunk->Render(commandBuffer, pipelineLayout);
            }
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

        for (auto& chunk : m_Chunks)
        {

            chunk->Update();
            
        }

        // Destroy all chunks that are to be destroyed if any
        DestroyDeletedChunks();
    }

    void DestroyDeletedChunks()
    {
        auto it = m_Chunks.begin();
        while (it != m_Chunks.end())
        {
            if ((*it)->IsDeleted())
            {
                (*it)->Destroy(m_Device);
                it = m_Chunks.erase(it); // Erase the current element and get the iterator to the next element
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
        for (auto& chunk : m_Chunks)
        {
            chunk->Destroy(m_Device);
        }
    }

    float GetChunkDeletionTime() const { return m_ChunkDeletionTime; }
private:
    const int m_ViewDistance{ 4 }; // View distance in grid tiles
    const float m_ChunkDeletionTime{ 3.f }; // Time to delete chunks after being marked for deletion

    std::vector<std::unique_ptr<Chunk>> m_Chunks;
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
        int minX = m_PlayerChunkPosition.x - m_ViewDistance;
        int maxX = m_PlayerChunkPosition.x + m_ViewDistance;
        int minZ = m_PlayerChunkPosition.z - m_ViewDistance;
        int maxZ = m_PlayerChunkPosition.z + m_ViewDistance;
        
        // Mark chunks for deletion outside the view distance
        for (auto& chunk : m_Chunks)
        {
            glm::ivec3 chunkPosition = CalculateChunkPosition(chunk->GetPosition());
            bool outsideViewDistance = chunkPosition.x < minX || chunkPosition.x > maxX || chunkPosition.z < minZ || chunkPosition.z > maxZ;
            if (outsideViewDistance)
            {
                chunk->SetIsMarkedForDeletion(true);
            }
            else
            {
                chunk->SetIsMarkedForDeletion(false);
            }
        }

        // Create chunks inside the view distance if not already existing
        for (int x = minX; x <= maxX; ++x)
        {
            for (int z = minZ; z <= maxZ; ++z)
            {
                glm::ivec3 chunkPosition = { x, 0, z };
                if (!IsChunkLoaded(chunkPosition))
                {
                    // Create and load the chunk
                    m_Chunks.emplace_back(std::move(std::make_unique<Chunk>(
                        glm::ivec3(
                            chunkPosition.x * Chunk::m_Width,
                            chunkPosition.y * Chunk::m_Height,
                            chunkPosition.z * Chunk::m_Depth),
                        m_Device,
                        m_PhysicalDevice,
                        m_CommandPool)));
                    m_Chunks.back()->GenerateMesh();
                }
            }
        }
    }

    bool IsChunkLoaded(const glm::ivec3& chunkPosition) const
    {
        // Check if a chunk at the given position is already loaded
        for (const auto& chunk : m_Chunks)
        {
            if (CalculateChunkPosition(chunk->GetPosition()) == chunkPosition)
            {
                return true;
            }
        }
        return false;
    }

private:
    ChunkGenerator() = default;
};