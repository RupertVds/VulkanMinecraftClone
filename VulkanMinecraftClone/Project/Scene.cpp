#include "Scene.h"

Scene::Scene(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool)
	:
	m_Device{ device },
	m_PhysicalDevice{ physicalDevice },
	m_CommandPool{ commandPool }
{
}

Scene::~Scene()
{}

void Scene::Render(VkCommandBuffer buffer) const
{
	for (auto& mesh : m_Meshes)
	{
		mesh->Draw(buffer);
	}
}

void Scene::Update() const
{
}

void Scene::AddCube()
{
}

void Scene::AddTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3)
{
	// Create a vector of vertices for the triangle
	std::vector<Vertex> vertices = { v1, v2, v3 };

	// Create a vector of indices for the triangle
	std::vector<uint16_t> indices = { 0, 1, 2 };

	// Create a new Mesh object with the vertices and indices, and add it to the vector of meshes
	m_Meshes.emplace_back(std::make_unique<Mesh>(vertices, indices, m_PhysicalDevice, m_Device, m_CommandPool));
}

void Scene::CleanUp()
{
	for (auto& mesh : m_Meshes)
	{
		mesh->DestroyMesh(m_Device);
	}
}

