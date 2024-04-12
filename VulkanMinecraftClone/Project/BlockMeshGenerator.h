#pragma once
#include <unordered_map>
#include <string>
#include <Texture.h>
#include "BlockMesh.h"

// IMPORTANT:
// ORDER OF APPEARANCE IN THE JSON FILE MUST MATCH!!!
enum class BlockType : unsigned short
{
    Grass,
    Stone,
    Dirt,
    Sand,
    Water,
    Log,
    Leaves
};

class BlockMesh;

class BlockMeshGenerator final
{
public:
    // Get the singleton instance
    static BlockMeshGenerator& GetInstance()
    {
        static BlockMeshGenerator instance;
        return instance;
    }

    void Init(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool);
    void Destroy(VkDevice device);
    // Generate block meshes based on texcoords
    void GenerateBlockMeshesFromAtlas(const std::string& jsonFilePath, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool);

    // Get block mesh for a specific block type
    //const BlockMesh& GetBlockMesh(const std::string& blockType);
    BlockMesh& GetBlockMesh(BlockType blockType);
    const std::vector<Texture>& GetTextures() const { return m_pTextures; }
private:
    std::unordered_map<BlockType, BlockMesh> m_BlockMeshes{};
    std::vector<Texture> m_pTextures{};

    // Private constructor to enforce singleton pattern
    BlockMeshGenerator() = default;
    ~BlockMeshGenerator() = default;
};
