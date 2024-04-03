#pragma once
#pragma once
#include <Mesh2D.h>

class Scene2D final
{
public:
	Scene2D(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool);
	~Scene2D();
	Scene2D(const Scene2D& other) = delete;
	Scene2D& operator=(const Scene2D& other) = delete;
	Scene2D(Scene2D&& other) = delete;
	Scene2D& operator=(Scene2D&& other) = delete;
public:
	void Render(VkCommandBuffer buffer) const;
	void Update() const;

	void AddTriangle(const Vertex2D& v1, const Vertex2D& v2, const Vertex2D& v3);
	void AddRectangle(const Vertex2D& topLeft, const Vertex2D& topRight, const Vertex2D& bottomRight, const Vertex2D& bottomLeft);
	void AddOval(const glm::vec2& position, float radiusX, float radiusY, uint32_t segments, const glm::vec3& color);
	void CleanUp();
private:
	std::vector<std::unique_ptr<Mesh2D>> m_Meshes;
	VkDevice m_Device;
	VkPhysicalDevice m_PhysicalDevice;
	VkCommandPool m_CommandPool;
};