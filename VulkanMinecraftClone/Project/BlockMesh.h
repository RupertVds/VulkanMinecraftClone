#pragma once
#include <glm\glm.hpp>
#include <vulkan\vulkan_core.h>
#include <vector>
#include <array>
#include <memory>

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoord;

	static std::unique_ptr<VkVertexInputBindingDescription> getBindingDescription()
	{
		auto bindingDescription = std::make_unique<VkVertexInputBindingDescription>();
		bindingDescription->binding = 0;
		bindingDescription->stride = sizeof(Vertex);
		bindingDescription->inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription; // std::move() ?
	}

	static std::unique_ptr<VkVertexInputAttributeDescription[]> getAttributeDescriptions()
	{
		auto attributeDescriptions = std::make_unique<VkVertexInputAttributeDescription[]>(3);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, normal);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions; // std::move() ?
	}
};

class CommandPool;

//enum class Direction : unsigned short
//{
//	Up,
//	Down,
//	North,
//	East,
//	South,
//	West
//};

class BlockMesh final
{
public:
	BlockMesh() = default;
	BlockMesh(
		const std::vector<Vertex>& vertices,
		const std::vector<uint16_t>& indices,
		VkPhysicalDevice physicalDevice,
		VkDevice device,
		VkCommandPool commandPool
	);
	~BlockMesh() = default;

	void DestroyMesh(VkDevice device);
	void Draw(VkCommandBuffer buffer, VkPipelineLayout pipelineLayout, const glm::vec3& translation) const;
	//void DrawFace(VkCommandBuffer buffer, VkPipelineLayout pipelineLayout, const glm::vec3& translation, Direction direction) const;
private:
	void Initialize(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool);
	void CreateVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool);
	void CreateIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool);

	VkBuffer m_VkVertexBuffer;
	VkDeviceMemory m_VkVertexBufferMemory;
	VkBuffer m_VkIndexBuffer;
	VkDeviceMemory m_VkIndexBufferMemory;
	std::vector<Vertex> m_Vertices;
	std::vector<uint16_t> m_Indices;
};