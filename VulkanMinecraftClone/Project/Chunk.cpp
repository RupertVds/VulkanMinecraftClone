#include "Chunk.h"
#include <ChunkGenerator.h>
#include <algorithm>

Chunk::Chunk(const glm::ivec3& position, SimplexNoise* noise, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool)
    :
    m_Position{ position },
    m_pNoise{ noise }
{
    m_Blocks.resize(m_Width * m_Height * m_Depth, BlockType::Air);
    // Move to chunkgenerator
    LoadBlockData("textures/blockdata.json");
    GenerateMesh();

    m_Device = device;
    // Create Vulkan buffers
    CreateVertexBuffer(device, physicalDevice, commandPool);
    CreateIndexBuffer(device, physicalDevice, commandPool);
}

void Chunk::GenerateMesh()
{
    // Generate mesh data for the chunk
    m_VerticesLand.clear();
    m_IndicesLand.clear();

    GenerateTerrain();

    // Create the optimized mesh
    for (int x = 0; x < m_Width; ++x)
    {
        for (int y = 0; y < m_Height; ++y)
        {
            for (int z = 0; z < m_Depth; ++z)
            {
                BlockType blockType = GetBlock({ x, y, z });

                // Skip air blocks
                if (blockType == BlockType::Air)
                {
                    continue;
                }

                // Add faces for opaque blocks
                for (const auto& [direction, offset] : m_FaceOffsets)
                {
                    int nx = x + offset.x;
                    int ny = y + offset.y;
                    int nz = z + offset.z;

                    if (blockType != BlockType::Leaves)
                    {
                        // Check if the neighboring block is the same type
                        if (IsSameBlockType(blockType, nx, ny, nz))
                        {
                            // Skip adding faces if the neighboring block is the same type
                            continue;
                        }
                    }

                    if (!IsOpaqueBlock(nx, ny, nz))
                    {
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

void Chunk::GenerateTerrain()
{
    for (int x = 0; x < m_Width; ++x)
    {
        for (int z = 0; z < m_Depth; ++z)
        {
            glm::ivec3 globalPosition = m_Position + glm::ivec3(x, 0, z);
            int height = ChunkGenerator::GetInstance().GetHeight(globalPosition);

            // Fill with water up to sea level
            for (int y = 0; y < m_Height * m_SeaLevel; ++y)
            {
                SetBlock(glm::ivec3(x, y, z), BlockType::Water);
            }

            // Make the base of the mountains sand inside the water
            if (height <= m_Height * m_SeaLevel)
            {
                // Fill up to the height with sand
                for (int y = 0; y <= height; ++y)
                {
                    SetBlock(glm::ivec3(x, y, z), BlockType::Sand);
                }
            }

            if (height > m_Height * m_SeaLevel)
            {
                
                // Calculate the number of layers of dirt
                int dirtLayers = min(height - (m_Height * m_SeaLevel + 1), 3);

                // Grass layer
                SetBlock(glm::ivec3(x, height, z), BlockType::GrassBlock);

                // Dirt layers
                for (int y = height - 1; y > height - dirtLayers - 1; --y)
                {
                    SetBlock(glm::ivec3(x, y, z), BlockType::Dirt);
                }

                // Stone below dirt layers
                for (int y = height - dirtLayers - 1; y >= 0; --y)
                {
                    SetBlock(glm::ivec3(x, y, z), BlockType::Stone);
                }
            }

            // Add a full layer of sand at the lowest position if height is zero
            SetBlock(glm::ivec3(x, 0, z), BlockType::Sand);
        }
    }
}





void Chunk::Update()
{
    if (m_IsMarkedForDeletion)
    {
        const float deltaTime = Timer::GetInstance().GetElapsed();

        // Increment deletion timer
        m_DeletionTimer += deltaTime;

        // If time for deletion destroy chunk
        if (m_DeletionTimer >= ChunkGenerator::GetInstance().GetChunkDeletionTime())
        {
            // do it with a boolean so the chunkgenerator can check and delete it properly while being notified
            m_IsDeleted = true;
        }
    }
}
