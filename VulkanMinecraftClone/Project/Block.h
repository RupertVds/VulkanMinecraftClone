#pragma once
#include <glm\glm.hpp>
#include "BlockMesh.h"
#include <glm\ext\matrix_transform.hpp>
#include <BlockMeshGenerator.h>
#include "Chunk.h"

//struct BlockData 
//{
//    glm::vec3 position;
//    BlockType blockType;
//};

class BlockMeshGenerator;

class Block final
{
public:
    Block(const BlockData& blockData) : m_BlockData(blockData) {}

    void Draw(VkCommandBuffer buffer, VkPipelineLayout pipelineLayout) const
    {
        //BlockMeshGenerator::GetInstance().GetBlockMesh(m_BlockData.blockType).Draw(buffer, pipelineLayout, m_BlockData.position);
        //BlockMeshGenerator::GetInstance().GetBlockMesh(m_BlockData.blockType).DrawFace(buffer, pipelineLayout, m_BlockData.position, m_BlockData.direction);

    }

private:
    BlockData m_BlockData;
};