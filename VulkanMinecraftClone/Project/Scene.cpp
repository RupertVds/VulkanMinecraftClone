#include "Scene.h"

Scene::~Scene()
{
}

void Scene::Render() const
{
	for (auto& mesh : m_Meshes)
	{
		mesh;
	}
}

void Scene::Update() const
{
}

void Scene::AddMesh(const Mesh& mesh)
{
}
