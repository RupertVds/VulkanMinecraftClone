#pragma once
#include <glm\glm.hpp>
#include <vulkan\vulkan_core.h>
#include <vector>
#include <array>
#include <memory>

struct Vertex2D
{
	glm::vec2 position;
	glm::vec3 color;

	static std::unique_ptr<VkVertexInputBindingDescription> getBindingDescription() 
	{
		auto bindingDescription = std::make_unique<VkVertexInputBindingDescription>();
		bindingDescription->binding = 0;
		bindingDescription->stride = sizeof(Vertex2D);
		bindingDescription->inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		
		return bindingDescription; // std::move() ?
	}

	static std::unique_ptr<VkVertexInputAttributeDescription[]> getAttributeDescriptions()
	{
		auto attributeDescriptions = std::make_unique<VkVertexInputAttributeDescription[]>(2);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex2D, position);
		
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex2D, color);

		return attributeDescriptions; // std::move() ?
	}
};

class CommandPool;

class Mesh2D final
{
public:
	Mesh2D(
		const std::vector<Vertex2D>& vertices, 
		const std::vector<uint16_t>& indices, 
		VkPhysicalDevice physicalDevice, 
		VkDevice device,
		VkCommandPool commandPool
	);
	~Mesh2D() = default;

	void DestroyMesh(VkDevice device);
	void Draw(VkCommandBuffer buffer);
private:
	void Initialize(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool);
	void CreateVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool);
	void CreateIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool);

	VkBuffer m_VkVertexBuffer;
	VkDeviceMemory m_VkVertexBufferMemory;
	VkBuffer m_VkIndexBuffer;
	VkDeviceMemory m_VkIndexBufferMemory;
	std::vector<Vertex2D> m_Vertices;
	std::vector<uint16_t> m_Indices;
};
	// all a mesh should contain:
	// m_vertices
	// m_indices
	// m_vertexbuffer
	// m_indexbuffer
	// remove everything else
	// special struct for specific data (for example world matrix of mesh)

	// for primitives
	// struct:
	// position
	// normal
	// texcoord
	
	// addtriangle method-> used to create faces
	// one with 3 indices
	// another overload with 3 indices and an offset
