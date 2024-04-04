#include "Scene.h"
#include <glm\ext\matrix_transform.hpp>

Scene::Scene(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool)
	:
	m_Device{ device },
	m_PhysicalDevice{ physicalDevice },
	m_CommandPool{ commandPool }
{
}

Scene::~Scene()
{}

void Scene::Render(VkCommandBuffer buffer, VkPipelineLayout pipelineLayout) const
{
	for (auto& mesh : m_Meshes)
	{
		mesh->Draw(buffer, pipelineLayout);
	}
}

void Scene::Update() const
{
}

void Scene::AddCube()
{
}

void Scene::AddTriangle(const glm::vec3& translation, const Vertex& v1, const Vertex& v2, const Vertex& v3)
{
	// Create a vector of vertices for the triangle
	std::vector<Vertex> vertices = { v1, v2, v3 };

	// Create a vector of indices for the triangle
	std::vector<uint16_t> indices = { 0, 1, 2 };

	// Create a new Mesh object with the vertices and indices, and add it to the vector of meshes
	m_Meshes.emplace_back(std::make_unique<Mesh>(vertices, indices, m_PhysicalDevice, m_Device, m_CommandPool));
	MeshData meshData{};
	meshData.model = glm::translate(glm::mat4{ 1.f }, translation); // Translate the identity matrix

	m_Meshes.back()->SetMeshData(meshData);
}

void Scene::CleanUp()
{
	for (auto& mesh : m_Meshes)
	{
		mesh->DestroyMesh(m_Device);
	}
}

