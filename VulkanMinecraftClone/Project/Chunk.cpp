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
                for (const auto& [direction, offset] : ChunkGenerator::GetInstance().GetFaceOffsets())
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
            if (height <= m_Height * m_SeaLevel + 3)
            {
                // Fill up to the height with sand
                for (int y = 0; y <= height; ++y)
                {
                    SetBlock(glm::ivec3(x, y, z), BlockType::Sand);
                }
            }

            if (height > m_Height * m_SeaLevel + 3)
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

void Chunk::AddFaceVertices(BlockType blockType, Direction direction, const glm::vec3& position)
{
    // Check if block data exists for the given block type
    auto it = ChunkGenerator::GetInstance().GetBlockData().find(blockType);
    if (it == ChunkGenerator::GetInstance().GetBlockData().end()) {
        // Handle error: Block data not found for the given block type
        std::cout << "ERROR: BLOCK DATA NOT FOUND FOR THE GIVEN BLOCK TYPE!\n";
        return;
    }

    const BlockData& blockData = it->second;

    constexpr float textureAtlasWidth = 16;
    constexpr float textureAtlasHeight = 16;

    // Get texture coordinates for the current face
    auto textureCoordsIt = blockData.textures.find(direction);
    if (textureCoordsIt == blockData.textures.end()) {
        std::cout << "ERROR: TEX COORDS NOT FOUND FOR THE CURRENT FACE DIRECTION!\n";
        return;
    }

    auto textureCoords = textureCoordsIt->second;

    float texCoordLeft = textureCoords.column * (1.0f / textureAtlasWidth);
    float texCoordRight = (textureCoords.column + 1) * (1.0f / textureAtlasWidth);
    float texCoordTop = textureCoords.row * (1.0f / textureAtlasHeight);
    float texCoordBottom = (textureCoords.row + 1) * (1.0f / textureAtlasHeight);

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
        faceVertices.push_back({ { basePosition.x - 0.5f, basePosition.y - 0.5f, basePosition.z }, normal, { texCoordRight, texCoordBottom } });
        faceVertices.push_back({ { basePosition.x - 0.5f, basePosition.y + 0.5f, basePosition.z }, normal, { texCoordRight, texCoordTop } });
        faceVertices.push_back({ { basePosition.x + 0.5f, basePosition.y + 0.5f, basePosition.z }, normal, { texCoordLeft, texCoordTop } });
        faceVertices.push_back({ { basePosition.x + 0.5f, basePosition.y - 0.5f, basePosition.z }, normal, { texCoordLeft, texCoordBottom } });
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
    size_t vertexOffset = m_VerticesLand.size();
    for (const Vertex& vertex : faceVertices)
    {
        m_VerticesLand.emplace_back(vertex);
    }

    // Add indices to the m_Indices vector
    m_IndicesLand.emplace_back(vertexOffset);
    m_IndicesLand.emplace_back(vertexOffset + 1);
    m_IndicesLand.emplace_back(vertexOffset + 2);
    m_IndicesLand.emplace_back(vertexOffset + 2);
    m_IndicesLand.emplace_back(vertexOffset + 3);
    m_IndicesLand.emplace_back(vertexOffset);
}
