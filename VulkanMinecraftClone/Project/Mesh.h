#pragma once
#include <glm\glm.hpp>
#include <vulkan\vulkan_core.h>
#include <vector>
#include <array>
#include <memory>

struct Vertex
{
	glm::vec3 position;
	glm::vec3 color;

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
		auto attributeDescriptions = std::make_unique<VkVertexInputAttributeDescription[]>(2);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);
		
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		return attributeDescriptions; // std::move() ?
	}
};

class Mesh final
{
public:
	Mesh(const std::vector<Vertex>& vertices);
	~Mesh() = default;
	
	void Initialize(VkPhysicalDevice physicalDevice, VkDevice device);
	void DestroyMesh(VkDevice device);
	void Draw(VkCommandBuffer buffer);
	void AddVertex(glm::vec2 pos, glm::vec3 color);
private:
	uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties memProperties, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	VkBuffer m_VkBuffer;
	VkDeviceMemory m_VkBufferMemory;
	std::vector<Vertex> m_Vertices;
};
