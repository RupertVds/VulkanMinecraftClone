#pragma once
#include <BlockMesh.h>
#include <vector>
#include <memory>
#include <Block.h>

class Scene final
{
public:
	enum class Direction
	{
		Up,
		Down,
		North,
		East,
		South,
		West
	};
public:
	Scene(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool);
	~Scene();
	Scene(const Scene& other) = delete;
	Scene& operator=(const Scene& other) = delete;
	Scene(Scene&& other) = delete;
	Scene& operator=(Scene&& other) = delete;
public:
	void Render(VkCommandBuffer buffer, VkPipelineLayout pipelineLayout) const;
	void Update() const;

	void AddBlock(const BlockData& blockData);
	void AddCube(const glm::vec3& position);
	void AddFace(const glm::vec3& position, Direction direction);
	void AddTriangle(const glm::vec3& translation, const Vertex& v1, const Vertex& v2, const Vertex& v3);

	void CleanUp();
private:
	std::vector<Block> m_Blocks;
	//std::unique_ptr<BlockMesh> m_BlockMesh;

	VkDevice m_Device;
	VkPhysicalDevice m_PhysicalDevice;
	VkCommandPool m_CommandPool;
};
