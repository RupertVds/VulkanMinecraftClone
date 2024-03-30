#include "Scene2D.h"

Scene2D::Scene2D(VkDevice device, VkPhysicalDevice physicalDevice)
	:
	m_Device{ device },
	m_PhysicalDevice{ physicalDevice }
{
}

Scene2D::~Scene2D()
{

}

void Scene2D::Render(VkCommandBuffer buffer) const
{
	// loop through all the meshes and render them with the same command buffer
	// that is passed
	for (auto& mesh : m_Meshes)
	{
		mesh->Draw(buffer);
	}
}

void Scene2D::Update() const
{
}

void Scene2D::AddTriangle(const Vertex2D& v1, const Vertex2D& v2, const Vertex2D& v3)
{
	// Create a vector of vertices for the triangle
	std::vector<Vertex2D> vertices = { v1, v2, v3 };

	// Create a new Mesh2D object with the vertices and add it to the vector of meshes
	m_Meshes.emplace_back(std::make_unique<Mesh2D>(vertices, m_PhysicalDevice, m_Device));
}

void Scene2D::CleanUp()
{
	// manually clean up all the meshes, this cannot be done by RAII since the vulkan device
	// is needed to delete these meshes
	// the vulkan device is manually deleted before VulkanBase (owner of all scenes) goes out of scope
	for (auto& mesh : m_Meshes)
	{
		mesh->DestroyMesh(m_Device);
	}
}
