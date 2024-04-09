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

void Scene::AddCube(const glm::vec3& position)
{
    AddFace(position, Direction::Up);
    AddFace(position, Direction::Down);
    AddFace(position, Direction::North);
    AddFace(position, Direction::East);
    AddFace(position, Direction::South);
    AddFace(position, Direction::West);
}

void Scene::AddFace(const glm::vec3& position, Direction direction)
{
    // Define the vertices and indices for the face based on the position and direction
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    switch (direction)
    {
    case Direction::Up:
        // Vertices for the face oriented upwards
        vertices = {
            {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},  // Top-left
            {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},   // Top-right
            {{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},    // Bottom-right
            {{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}    // Bottom-left
        };
        indices = { 0, 1, 2, 2, 3, 0 }; // Indices for the face
        break;

    case Direction::Down:
        // Vertices for the face oriented downwards
        vertices = {
            {{-0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},   // Top-left
            {{0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},    // Top-right
            {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},  // Bottom-right
            {{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}}  // Bottom-left
        };
        indices = { 0, 1, 2, 2, 3, 0 }; // Indices for the face
        break;

    case Direction::North:
        // Vertices for the face oriented towards the north
        vertices = {
            {{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},    // Top-left
            {{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},     // Top-right
            {{0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},    // Bottom-right
            {{-0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}    // Bottom-left
        };
        indices = { 0, 1, 2, 2, 3, 0 }; // Indices for the face
        break;

    case Direction::South:
        // Vertices for the face oriented towards the south
        vertices = {
            {{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},  // Top-left
            {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},   // Top-right
            {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},    // Bottom-right
            {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}}    // Bottom-left
        };
        indices = { 0, 1, 2, 2, 3, 0 }; // Indices for the face
        break;

    case Direction::East:
        // Vertices for the face oriented towards the east
        vertices = {
            {{0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},    // Top-left
            {{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},     // Top-right
            {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},    // Bottom-right
            {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}}    // Bottom-left
        };
        indices = { 0, 1, 2, 2, 3, 0 }; // Indices for the face
        break;

    case Direction::West:
        // Vertices for the face oriented towards the west
        vertices = {
            {{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},  // Top-left
            {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},   // Top-right
            {{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},    // Bottom-right
            {{-0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}    // Bottom-left
        };
        indices = { 0, 1, 2, 2, 3, 0 }; // Indices for the face
        break;
    }

    // Create a new Mesh object with the vertices and indices, and add it to the vector of meshes
    m_Meshes.emplace_back(std::make_unique<Mesh>(vertices, indices, m_PhysicalDevice, m_Device, m_CommandPool));
    MeshData meshData{};
    meshData.model = glm::translate(glm::mat4{ 1.f }, position); // Translate the identity matrix by the specified position

    m_Meshes.back()->SetMeshData(meshData);
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

