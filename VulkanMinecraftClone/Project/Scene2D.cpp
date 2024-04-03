#include "Scene2D.h"
#include <glm/glm.hpp>
#include <cmath> // For std::cos, std::sin

// Define pi manually
constexpr float PI = 3.14159265358979323846f;

Scene2D::Scene2D(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool)
	:
	m_Device{ device },
	m_PhysicalDevice{ physicalDevice },
	m_CommandPool{ commandPool }
{
}

Scene2D::~Scene2D()
{}

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

	// Create a vector of indices for the triangle
	std::vector<uint16_t> indices = { 0, 1, 2 };

	// Create a new Mesh2D object with the vertices and indices, and add it to the vector of meshes
	//m_Meshes.emplace_back(std::make_unique<Mesh2D>(vertices, indices, m_PhysicalDevice, m_Device));
	m_Meshes.emplace_back(std::make_unique<Mesh2D>(vertices, indices, m_PhysicalDevice, m_Device, m_CommandPool));
}

void Scene2D::AddRectangle(const Vertex2D& topLeft, const Vertex2D& topRight, const Vertex2D& bottomRight, const Vertex2D& bottomLeft)
{
	std::vector<Vertex2D> vertices = { topLeft, topRight, bottomRight, bottomLeft };
	std::vector<uint16_t> indices = { 0, 1, 2, 2, 3, 0 };

	m_Meshes.emplace_back(std::make_unique<Mesh2D>(vertices, indices, m_PhysicalDevice, m_Device, m_CommandPool));
}

void Scene2D::AddOval(const glm::vec2& position, float radiusX, float radiusY, uint32_t segments, const glm::vec3& color)
{
	std::vector<Vertex2D> vertices;
	std::vector<uint16_t> indices;
	float thetaIncrement = 2.0f * PI / static_cast<float>(segments);

	// Center vertex
	vertices.emplace_back(Vertex2D{ position, color });

	// Generate vertices along the ellipse
	for (uint16_t i = 0; i <= segments; ++i) {
		float theta = i * thetaIncrement;
		float x = position.x + radiusX * std::cos(theta);
		float y = position.y + radiusY * std::sin(theta);
		vertices.emplace_back(Vertex2D{ glm::vec2(x, y), color });
	}

	// Generate indices for triangles covering the oval area
	for (uint16_t i = 1; i < segments; ++i) {
		indices.emplace_back(0); // Center vertex
		indices.emplace_back(i);
		indices.emplace_back(i + 1);
	}
	// Connect last vertex to the first one
	indices.emplace_back(0); // Center vertex
	indices.emplace_back(segments);
	indices.emplace_back(1);

	m_Meshes.emplace_back(std::make_unique<Mesh2D>(vertices, indices, m_PhysicalDevice, m_Device, m_CommandPool));
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
