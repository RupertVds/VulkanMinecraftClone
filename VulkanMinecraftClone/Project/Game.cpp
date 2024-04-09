#include "Game.h"
#include <Timer.h>
#include <Camera.h>
#include <InputManager.h>
#include <iostream>

void Game::Init(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool)
{
	Camera::GetInstance().Init(&InputManager::GetInstance(), { 0, 0, 5 });

	m_pScene2D = std::make_unique<Scene2D>(device, physicalDevice, commandPool);

	m_pScene2D->AddTriangle(
		{ {-0.95f, 0.9f}, {1.0f, 0.0f, 0.0f} },
		{ {-0.9f, 0.80f}, {0.0f, 1.0f, 0.0f} },
		{ {-0.85f, 0.9f}, {0.0f, 0.0f, 1.0f} });

	m_pScene2D->AddTriangle(
		{ {-0.80f, 0.9f}, {1.0f, 0.0f, 1.0f} },
		{ {-0.75f, 0.80f}, {1.0f, 1.0f, 0.0f} },
		{ {-0.70f, 0.9f}, {0.0f, 1.0f, 1.0f} });

	m_pScene2D->AddOval({ -0.90f, 0.7f }, 0.05f, 0.05f, 50, { 0.25f, 1.f, 0.25f });

	m_pScene2D->AddRectangle(
		{ {-0.80f, 0.65f}, {1.0f, 0.0f, 0.0f} },
		{ {-0.70f, 0.65f}, {0.0f, 1.0f, 0.0f} },
		{ {-0.70f, 0.75f}, {0.0f, 0.0f, 1.0f} },
		{ {-0.80f, 0.75f}, {1.0f, 1.0f, 1.0f} });

	m_pScene3D = std::make_unique<Scene>(device, physicalDevice, commandPool);

	m_pScene3D->AddFace({ 0,0,0 }, Scene::Direction::Up);
	m_pScene3D->AddFace({ 0,0.1f,0 }, Scene::Direction::Up);
	//m_pScene3D->AddFace({ 0,0,0 }, Scene::Direction::Down);
	//m_pScene3D->AddFace({ 0,0,0 }, Scene::Direction::North);
	//m_pScene3D->AddFace({ 0,0,0 }, Scene::Direction::East);
	//m_pScene3D->AddFace({ 0,0,0 }, Scene::Direction::West);
	//m_pScene3D->AddFace({ 0,0,0 }, Scene::Direction::South);

	//m_pScene3D->AddCube({ 0,1,0 });
	//m_pScene3D->AddCube({ 0,2,0 });
	//m_pScene3D->AddCube({ 0,3,0 });
	//m_pScene3D->AddCube({ 1,3,0 });
	//m_pScene3D->AddCube({ 2,3,0 });
	//m_pScene3D->AddCube({ 3,3,0 });
	//m_pScene3D->AddCube({ 4,3,0 });
	//m_pScene3D->AddCube({ 5,3,0 });
	//m_pScene3D->AddCube({ 6,3,0 });
	//m_pScene3D->AddCube({ 7,3,0 });
	//m_pScene3D->AddCube({ 8,3,0 });
	//m_pScene3D->AddCube({ 9,3,0 });
	m_pScene3D->AddCube({ 10,3,0 });

	//// Define the vertices for the cube
	//std::vector<Vertex> vertices = 
	//{
	//	// Front face
	//	{{-1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},   // 0: Bottom-left
	//	{{1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},    // 1: Bottom-right
	//	{{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},     // 2: Top-right
	//	{{-1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.0f}},    // 3: Top-left

	//	// Back face
	//	{{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 1.0f}},  // 4: Bottom-left
	//	{{1.0f, -1.0f, -1.0f}, {0.0f, 1.0f, 1.0f}},   // 5: Bottom-right
	//	{{1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}},    // 6: Top-right
	//	{{-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}},   // 7: Top-left
	//};

	//// Define the indices for the cube
	////std::vector<uint16_t> indices = {
	////	// Front face
	////	0, 1, 2,  // Triangle 1
	////	2, 3, 0,  // Triangle 2
	////	// Right face
	////	1, 5, 6,  // Triangle 1
	////	6, 2, 1,  // Triangle 2
	////	// Back face
	////	7, 6, 5,  // Triangle 1
	////	5, 4, 7,  // Triangle 2
	////	// Left face
	////	4, 0, 3,  // Triangle 1
	////	3, 7, 4,  // Triangle 2
	////	// Top face
	////	3, 2, 6,  // Triangle 1
	////	6, 7, 3,  // Triangle 2
	////	// Bottom face
	////	4, 5, 1,  // Triangle 1
	////	1, 0, 4   // Triangle 2
	////};

	//// Add the cube to the scene
	//m_pScene3D->AddTriangle({ 0, 0, 0 }, vertices[0], vertices[1], vertices[2]); // Front face
	//m_pScene3D->AddTriangle({ 0, 0, 0 }, vertices[0], vertices[2], vertices[3]);
	//m_pScene3D->AddTriangle({ 0, 0, 0 }, vertices[1], vertices[5], vertices[6]); // Right face
	//m_pScene3D->AddTriangle({ 0, 0, 0 }, vertices[1], vertices[6], vertices[2]);
	//m_pScene3D->AddTriangle({ 0, 0, 0 }, vertices[7], vertices[6], vertices[5]); // Back face
	//m_pScene3D->AddTriangle({ 0, 0, 0 }, vertices[7], vertices[5], vertices[4]);
	//m_pScene3D->AddTriangle({ 0, 0, 0 }, vertices[4], vertices[0], vertices[3]); // Left face
	//m_pScene3D->AddTriangle({ 0, 0, 0 }, vertices[4], vertices[3], vertices[7]);
	//m_pScene3D->AddTriangle({ 0, 0, 0 }, vertices[3], vertices[2], vertices[6]); // Top face
	//m_pScene3D->AddTriangle({ 0, 0, 0 }, vertices[3], vertices[6], vertices[7]);
	//m_pScene3D->AddTriangle({ 0, 0, 0 }, vertices[4], vertices[1], vertices[0]); // Bottom face
	//m_pScene3D->AddTriangle({ 0, 0, 0 }, vertices[4], vertices[5], vertices[1]);
}

void Game::Update()
{
	Timer::GetInstance().Update();
	m_PrintTimer += Timer::GetInstance().GetElapsed();
	if (m_PrintTimer >= m_PrintDelay)
	{
		m_PrintTimer = 0.f;
		std::cout << "dFPS: " << Timer::GetInstance().GetdFPS() << std::endl;
	}

	Camera::GetInstance().Update(Timer::GetInstance().GetElapsed());
	if (InputManager::GetInstance().IsKeyPressed(GLFW_KEY_C))
	{
		InputManager::GetInstance().ToggleFPSMode();
	}

	// Do game update stuff
	m_pScene2D->Update();
	m_pScene3D->Update();
}

void Game::Render(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{
	m_pScene3D->Render(commandBuffer, pipelineLayout);
}

void Game::Render2D(VkCommandBuffer commandBuffer)
{
	m_pScene2D->Render(commandBuffer);
}

void Game::Destroy()
{
	m_pScene2D->CleanUp();
	m_pScene3D->CleanUp();
}
