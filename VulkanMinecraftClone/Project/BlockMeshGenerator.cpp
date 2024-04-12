#include "BlockMeshGenerator.h"
#include <fstream>
#include <glm\glm.hpp>
// Json library used: https://github.com/nlohmann/json
#include <vendor/json.hpp>

void BlockMeshGenerator::Init(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool)
{
    Texture textureAtlas{};
    textureAtlas.Init(device, physicalDevice, commandPool);
    m_pTextures.push_back(textureAtlas);

    GenerateBlockMeshesFromAtlas("textures/blockdata.json", device, physicalDevice, commandPool);
}

void BlockMeshGenerator::Destroy(VkDevice device)
{
    for (auto& texture : m_pTextures)
    {
        texture.Destroy(device);
    }

    for (auto& blockMesh : m_BlockMeshes)
    {
        blockMesh.second.DestroyMesh(device);
    }
}


void BlockMeshGenerator::GenerateBlockMeshesFromAtlas(const std::string& jsonFilePath, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool) 
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

    // Enum value counter
    unsigned short enumValue = 0;

    // Iterate over each block type
    for (const auto& block : blocks) {
        // Extract block data
        std::string id = block["id"];
        int row = block["row"];
        int column = block["col"];

        // TODO: remove hardcoded
        float textureAtlasWidth = 16.f;
        float textureAtlasHeight = 16.f;

        // Calculate texture coordinates
        float texCoordLeft = column * (1.0f / textureAtlasWidth);
        float texCoordRight = (column + 1) * (1.0f / textureAtlasWidth);
        float texCoordTop = row * (1.0f / textureAtlasHeight);
        float texCoordBottom = (row + 1) * (1.0f / textureAtlasHeight);

        // Generate vertices with adjusted texture coordinates
        std::vector<Vertex> vertices{
            // Front face vertices
            { { -0.5f, -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f }, { texCoordLeft, texCoordTop } },     // Bottom-left
            { {0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {texCoordRight, texCoordTop} },      // Bottom-right
            { {0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {texCoordRight, texCoordBottom} },   // Top-right
            { {-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {texCoordLeft, texCoordBottom} },    // Top-left

            // Back face vertices
            { {-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {texCoordRight, texCoordTop} },    // Bottom-left
            { {0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {texCoordLeft, texCoordTop} },     // Bottom-right
            { {0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {texCoordLeft, texCoordBottom} },  // Top-right
            { {-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {texCoordRight, texCoordBottom} },   // Top-left
            //            // Back face vertices
            //{ {-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {texCoordLeft, texCoordTop} },    // Bottom-left
            //{ {0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {texCoordRight, texCoordTop} },     // Bottom-right
            //{ {0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {texCoordRight, texCoordBottom} },  // Top-right
            //{ {-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {texCoordLeft, texCoordBottom} },   // Top-left
            // Right face vertices
            { {0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {texCoordLeft, texCoordTop} },      // Bottom-left
            { {0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {texCoordRight, texCoordTop} },    // Bottom-right
            { {0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {texCoordRight, texCoordBottom} },  // Top-right
            { {0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {texCoordLeft, texCoordBottom} },    // Top-left

            // Left face vertices
            { {-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {texCoordLeft, texCoordTop} },   // Bottom-left
            { {-0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {texCoordRight, texCoordTop} },   // Bottom-right
            { {-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {texCoordRight, texCoordBottom} }, // Top-right
            { {-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {texCoordLeft, texCoordBottom} }, // Top-left
            
            // Top face vertices
            { {-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {texCoordLeft, texCoordTop} },    // Bottom-left
            { {0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {texCoordRight, texCoordTop} },     // Bottom-right
            { {0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {texCoordRight, texCoordBottom} }, // Top-right
            { {-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {texCoordLeft, texCoordBottom} },  // Top-left
            
            // Bottom face vertices
            { {-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {texCoordLeft, texCoordTop} },  // Bottom-left
            { {0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {texCoordRight, texCoordTop} },   // Bottom-right
            { {0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {texCoordRight, texCoordBottom} }, // Top-right
            { {-0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {texCoordLeft, texCoordBottom} }   // Top-left
        };

        std::vector<uint16_t> indices = {
            // Front face
            0, 1, 2,  2, 3, 0,
            // Back face
            4, 7, 6,  6, 5, 4,
            //4, 5, 6,  6, 7, 4,
            // Right face
            8, 9, 10,  10, 11, 8,
            // Left face
            12, 13, 14,  14, 15, 12,
            // Top face
            16, 17, 18,  18, 19, 16,
            // Bottom face
            20, 21, 22,  22, 23, 20
        };

        // Create a new Mesh object using the generated vertices and indices
        // and store it in the container
        BlockType blockType = static_cast<BlockType>(enumValue);
        m_BlockMeshes[blockType] = BlockMesh(vertices, indices, physicalDevice, device, commandPool);

        // Increment enum value for the next block type
        ++enumValue;
    }
}

BlockMesh& BlockMeshGenerator::GetBlockMesh(BlockType blockType)
{
    return m_BlockMeshes[blockType];
}