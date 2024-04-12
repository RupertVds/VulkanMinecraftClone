#include "Game.h"
#include <Timer.h>
#include <Camera.h>
#include <InputManager.h>
#include <iostream>

void Game::Init(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool)
{
	Camera::GetInstance().Init(&InputManager::GetInstance(), { 0, 0, 5 });

	//Texture textureAtlas{};
	//textureAtlas.Init(device, physicalDevice, commandPool);
	//m_pTextures.push_back(textureAtlas);

	m_pScene3D = std::make_unique<Scene>(device, physicalDevice, commandPool);


	//m_pScene3D->AddFace({ 0,0,0 }, Scene::Direction::Up);
	//m_pScene3D->AddFace({ 0,0.1f,0 }, Scene::Direction::Up);
	//m_pScene3D->AddFace({ 0,0,0 }, Scene::Direction::Down);
	//m_pScene3D->AddFace({ 0,0,0 }, Scene::Direction::North);
	//m_pScene3D->AddFace({ 0,0,0 }, Scene::Direction::East);
	//m_pScene3D->AddFace({ 0,0,0 }, Scene::Direction::West);
	//m_pScene3D->AddFace({ 0,0,0 }, Scene::Direction::South);

	m_pScene3D->AddBlock({ { 0,0,0 }, BlockType::Grass });
	//m_pScene3D->AddBlock({ { 0,0,-1 }, BlockType::Grass });
	//m_pScene3D->AddBlock({ { 1,0,-1 }, BlockType::Grass });
	//m_pScene3D->AddBlock({ { 1,0,0 }, BlockType::Grass });
	//m_pScene3D->AddBlock({ { 2,0,0 }, BlockType::Dirt });
	//m_pScene3D->AddBlock({ { 3,0,0 }, BlockType::Stone });


	//for (int x{}; x < 16; ++x)
	//{
	//	for (int y{}; y < 16; ++y)
	//	{
	//		for (int z{}; z < 16; ++z)
	//		{
	//			m_pScene3D->AddBlock({ { x,y,z }, BlockType::Dirt });
	//		}
	//	}
	//}

#pragma region 2D
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

#pragma endregion
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

void Game::Destroy(VkDevice device)
{
	m_pScene2D->CleanUp();
	m_pScene3D->CleanUp();
	//m_pTextureAtlas->Destroy(device);
	//for (auto& texture : m_pTextures)
	//{
	//	texture.Destroy(device);
	//}
}
