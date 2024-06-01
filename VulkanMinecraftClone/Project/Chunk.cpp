#include "Chunk.h"
#include <ChunkGenerator.h>
#include <algorithm>
#include "GraphicsPipeline3D.h"
#include <random>

const int TREE_HEIGHT = 5;
const int TREE_TRUNK_HEIGHT = 4;
const int TREE_LEAF_WIDTH = 5;
const int TREE_LEAF_HEIGHT = 4;
const int TREE_LEAF_MIDDLE_HEIGHT = 2;
const int TREE_LEAF_MIDDLE_WIDTH = 3;

struct Tree
{
    glm::ivec3 trunk[TREE_TRUNK_HEIGHT];
    glm::ivec3 leaves[TREE_LEAF_WIDTH][TREE_LEAF_HEIGHT][TREE_LEAF_WIDTH];

    Tree(const glm::ivec3& basePosition)
    {
        // Initialize trunk
        for (int i = 0; i < TREE_TRUNK_HEIGHT; ++i)
        {
            trunk[i] = basePosition + glm::ivec3(0, i, 0);
        }

        // Initialize leaves
        int halfWidth = TREE_LEAF_WIDTH / 2;
        int halfMiddleWidth = TREE_LEAF_MIDDLE_WIDTH / 2;
        for (int x = -halfWidth; x <= halfWidth; ++x)
        {
            for (int y = 0; y < TREE_LEAF_HEIGHT; ++y)
            {
                for (int z = -halfWidth; z <= halfWidth; ++z)
                {
                    if (y < TREE_LEAF_MIDDLE_HEIGHT) {
                        leaves[x + halfWidth][y][z + halfWidth] = basePosition + glm::ivec3(x, TREE_TRUNK_HEIGHT + y - 1, z);
                    }
                    else {
                        leaves[x + halfWidth][y][z + halfWidth] = basePosition + glm::ivec3(x, TREE_TRUNK_HEIGHT + TREE_LEAF_MIDDLE_HEIGHT - 1, z);
                    }
                }
            }
        }
    }
};




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
    //CreateVertexBuffer(device, physicalDevice, commandPool);
    //CreateIndexBuffer(device, physicalDevice, commandPool);
    CreateLandVertexBuffer(device, physicalDevice, commandPool);
    CreateLandIndexBuffer(device, physicalDevice, commandPool);
    if (!m_VerticesWater.empty())
    {
        CreateWaterVertexBuffer(device, physicalDevice, commandPool);
        CreateWaterIndexBuffer(device, physicalDevice, commandPool);
    }
}

void Chunk::GenerateMesh()
{
    // Generate mesh data for the chunk
    m_VerticesLand.clear();
    m_IndicesLand.clear();
    m_VerticesWater.clear();
    m_IndicesWater.clear();

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
                        if (blockType == BlockType::Water)
                        {
                            AddFaceVertices(m_VerticesWater, m_IndicesWater, blockType, direction, glm::vec3(x, y, z));
                        }
                        else
                        {
                            AddFaceVertices(m_VerticesLand, m_IndicesLand, blockType, direction, glm::vec3(x, y, z));
                        }
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
                int dirtLayers = (((height - (m_Height * m_SeaLevel + 1)) < (3)) ? (height - (m_Height * m_SeaLevel + 1)) : (3));

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
        }
    }

    // Tree generation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    const float treeSpawnChance = 0.05f; // 5% chance to spawn a tree on each grass block

    for (int x = 0; x < m_Width; ++x)
    {
        for (int z = 0; z < m_Depth; ++z)
        {
            glm::ivec3 globalPosition = m_Position + glm::ivec3(x, 0, z);
            int height = ChunkGenerator::GetInstance().GetHeight(globalPosition);
            glm::ivec3 grassBlockPosition = glm::ivec3(x, height, z);

            if (GetBlock(grassBlockPosition) == BlockType::GrassBlock)
            {
                // Check if there's enough space for a tree
                bool canPlaceTree = true;
                Tree potentialTree(grassBlockPosition + glm::ivec3(0, 1, 0));
                for (const auto& trunkPos : potentialTree.trunk)
                {
                    if (!IsWithinBounds(trunkPos) || GetBlock(trunkPos) != BlockType::Air)
                    {
                        canPlaceTree = false;
                        break;
                    }
                }
                if (canPlaceTree)
                {
                    for (int dx = 0; dx < TREE_LEAF_WIDTH; ++dx)
                    {
                        for (int dy = 0; dy < TREE_LEAF_HEIGHT; ++dy)
                        {
                            for (int dz = 0; dz < TREE_LEAF_WIDTH; ++dz)
                            {
                                glm::ivec3 leafPos = potentialTree.leaves[dx][dy][dz];
                                if (!IsWithinBounds(leafPos) || GetBlock(leafPos) != BlockType::Air)
                                {
                                    canPlaceTree = false;
                                    break;
                                }
                            }
                            if (!canPlaceTree) break;
                        }
                        if (!canPlaceTree) break;
                    }
                }

                // Place the tree if conditions are met
                if (canPlaceTree && dis(gen) < treeSpawnChance)
                {
                    for (const auto& trunkPos : potentialTree.trunk)
                    {
                        SetBlock(trunkPos, BlockType::Log);
                    }
                    // Set leaves according to specified dimensions
                    for (int dx = 0; dx < TREE_LEAF_WIDTH; ++dx)
                    {
                        for (int dy = 0; dy < TREE_LEAF_HEIGHT; ++dy)
                        {
                            for (int dz = 0; dz < TREE_LEAF_WIDTH; ++dz)
                            {
                                // Check if leaf position is within the specified shape
                                if (dy < TREE_LEAF_MIDDLE_HEIGHT ||
                                    (dx >= (TREE_LEAF_WIDTH - TREE_LEAF_MIDDLE_WIDTH) / 2 &&
                                        dx < (TREE_LEAF_WIDTH + TREE_LEAF_MIDDLE_WIDTH) / 2 &&
                                        dz >= (TREE_LEAF_WIDTH - TREE_LEAF_MIDDLE_WIDTH) / 2 &&
                                        dz < (TREE_LEAF_WIDTH + TREE_LEAF_MIDDLE_WIDTH) / 2))
                                {
                                    SetBlock(potentialTree.leaves[dx][dy][dz], BlockType::Leaves);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

bool Chunk::IsWithinBounds(const glm::ivec3& position) const
{
    return position.x >= 0 && position.x < m_Width &&
        position.y >= 0 && position.y < m_Height &&
        position.z >= 0 && position.z < m_Depth;
}

void Chunk::RenderLand(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{
    // Bind vertex buffer
    VkBuffer vertexBuffers[] = { m_VertexBufferLand };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    // Bind index buffer
    vkCmdBindIndexBuffer(commandBuffer, m_IndexBufferLand, 0, VK_INDEX_TYPE_UINT32);

    // Update push constants
    PushConstants pushConstants{};

    pushConstants.translation = m_Position;
    pushConstants.time = Timer::GetInstance().GetElapsed();
    vkCmdPushConstants(
        commandBuffer,
        pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(PushConstants),
        &pushConstants
    );

    // Submit rendering commands
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_IndicesLand.size()), 1, 0, 0, 0);
}

void Chunk::RenderWater(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{
    if (m_VerticesWater.empty()) return;

    // Bind vertex buffer
    VkBuffer vertexBuffers[] = { m_VertexBufferWater };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    // Bind index buffer
    vkCmdBindIndexBuffer(commandBuffer, m_IndexBufferWater, 0, VK_INDEX_TYPE_UINT32);

    // Update push constants
    PushConstants pushConstants{};

    pushConstants.translation = m_Position;
    pushConstants.time = ChunkGenerator::GetInstance().GetWaterTimer();
    vkCmdPushConstants(
        commandBuffer,
        pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(PushConstants),
        &pushConstants
    );

    // Draw indexed
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_IndicesWater.size()), 1, 0, 0, 0);
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

    //test += Timer::GetInstance().GetElapsed();;
}

void Chunk::AddFaceVertices(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, BlockType blockType, Direction direction, const glm::vec3& position)
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
        faceVertices.emplace_back(Vertex{ { basePosition.x - 0.5f, basePosition.y, basePosition.z + 0.5f }, normal, { texCoordLeft, texCoordBottom } });
        faceVertices.emplace_back(Vertex{ { basePosition.x + 0.5f, basePosition.y, basePosition.z + 0.5f }, normal, { texCoordRight, texCoordBottom } });
        faceVertices.emplace_back(Vertex{ { basePosition.x + 0.5f, basePosition.y, basePosition.z - 0.5f }, normal, { texCoordRight, texCoordTop } });
        faceVertices.emplace_back(Vertex{ { basePosition.x - 0.5f, basePosition.y, basePosition.z - 0.5f }, normal, { texCoordLeft, texCoordTop } });
        break;

    case Direction::Down:
        faceVertices.emplace_back(Vertex{ { basePosition.x - 0.5f, basePosition.y, basePosition.z - 0.5f }, normal, { texCoordLeft, texCoordTop } });
        faceVertices.emplace_back(Vertex{ { basePosition.x + 0.5f, basePosition.y, basePosition.z - 0.5f }, normal, { texCoordRight, texCoordTop } });
        faceVertices.emplace_back(Vertex{ { basePosition.x + 0.5f, basePosition.y, basePosition.z + 0.5f }, normal, { texCoordRight, texCoordBottom } });
        faceVertices.emplace_back(Vertex{ { basePosition.x - 0.5f, basePosition.y, basePosition.z + 0.5f }, normal, { texCoordLeft, texCoordBottom } });
        break;

    case Direction::North:
        faceVertices.emplace_back(Vertex{ { basePosition.x - 0.5f, basePosition.y - 0.5f, basePosition.z }, normal, { texCoordRight, texCoordBottom } });
        faceVertices.emplace_back(Vertex{ { basePosition.x - 0.5f, basePosition.y + 0.5f, basePosition.z }, normal, { texCoordRight, texCoordTop } });
        faceVertices.emplace_back(Vertex{ { basePosition.x + 0.5f, basePosition.y + 0.5f, basePosition.z }, normal, { texCoordLeft, texCoordTop } });
        faceVertices.emplace_back(Vertex{ { basePosition.x + 0.5f, basePosition.y - 0.5f, basePosition.z }, normal, { texCoordLeft, texCoordBottom } });
        break;

    case Direction::South:
        faceVertices.emplace_back(Vertex{ { basePosition.x + 0.5f, basePosition.y - 0.5f, basePosition.z }, normal, { texCoordRight, texCoordBottom } });
        faceVertices.emplace_back(Vertex{ { basePosition.x + 0.5f, basePosition.y + 0.5f, basePosition.z }, normal, { texCoordRight, texCoordTop } });
        faceVertices.emplace_back(Vertex{ { basePosition.x - 0.5f, basePosition.y + 0.5f, basePosition.z }, normal, { texCoordLeft, texCoordTop } });
        faceVertices.emplace_back(Vertex{ { basePosition.x - 0.5f, basePosition.y - 0.5f, basePosition.z }, normal, { texCoordLeft, texCoordBottom } });
        break;

    case Direction::East:
        faceVertices.emplace_back(Vertex{ { basePosition.x, basePosition.y - 0.5f, basePosition.z + 0.5f }, normal, { texCoordLeft, texCoordBottom } });
        faceVertices.emplace_back(Vertex{ { basePosition.x, basePosition.y - 0.5f, basePosition.z - 0.5f }, normal, { texCoordRight, texCoordBottom } });
        faceVertices.emplace_back(Vertex{ { basePosition.x, basePosition.y + 0.5f, basePosition.z - 0.5f }, normal, { texCoordRight, texCoordTop } });
        faceVertices.emplace_back(Vertex{ { basePosition.x, basePosition.y + 0.5f, basePosition.z + 0.5f }, normal, { texCoordLeft, texCoordTop } });
        break;

    case Direction::West:
        faceVertices.emplace_back(Vertex{ glm::vec3{ basePosition.x, basePosition.y - 0.5f, basePosition.z - 0.5f }, normal, { texCoordLeft, texCoordBottom } });
        faceVertices.emplace_back(Vertex{ glm::vec3{ basePosition.x, basePosition.y - 0.5f, basePosition.z + 0.5f }, normal, { texCoordRight, texCoordBottom } });
        faceVertices.emplace_back(Vertex{ glm::vec3{ basePosition.x, basePosition.y + 0.5f, basePosition.z + 0.5f }, normal, { texCoordRight, texCoordTop } });
        faceVertices.emplace_back(Vertex{ glm::vec3{ basePosition.x, basePosition.y + 0.5f, basePosition.z - 0.5f }, normal, { texCoordLeft, texCoordTop } });
        break;
    }

    // Add vertices to the provided vertices vector
    // Add vertices to the provided vertices vector
    size_t vertexOffset = vertices.size();
    vertices.insert(vertices.end(), faceVertices.begin(), faceVertices.end());

    // Add indices to the provided indices vector
    indices.emplace_back(vertexOffset);
    indices.emplace_back(vertexOffset + 1);
    indices.emplace_back(vertexOffset + 2);
    indices.emplace_back(vertexOffset + 2);
    indices.emplace_back(vertexOffset + 3);
    indices.emplace_back(vertexOffset);
}
