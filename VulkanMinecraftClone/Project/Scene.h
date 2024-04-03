#pragma once
#include <Mesh.h>
#include <vector>
#include <memory>

class Scene final
{
public:
	Scene(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool);
	~Scene();
	Scene(const Scene& other) = delete;
	Scene& operator=(const Scene& other) = delete;
	Scene(Scene&& other) = delete;
	Scene& operator=(Scene&& other) = delete;
public:
	void Render(VkCommandBuffer buffer) const;
	void Update() const;

	void AddCube();
	void AddTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3);

	void CleanUp();
private:
	std::vector<std::unique_ptr<Mesh>> m_Meshes;
	VkDevice m_Device;
	VkPhysicalDevice m_PhysicalDevice;
	VkCommandPool m_CommandPool;
};
