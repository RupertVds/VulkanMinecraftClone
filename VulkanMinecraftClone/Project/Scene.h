#pragma once
#include <Mesh.h>
#include <vector>
#include <memory>

class Scene final
{
public:
	Scene() = default;
	~Scene();
	Scene(const Scene& other) = delete;
	Scene& operator=(const Scene& other) = delete;
	Scene(Scene&& other) = delete;
	Scene& operator=(Scene&& other) = delete;
public:
	void Render() const;
	void Update() const;
	void AddMesh(const Mesh& mesh);
private:
	std::vector<std::unique_ptr<Mesh>> m_Meshes;
};
