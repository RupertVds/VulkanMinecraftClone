#include "Game.h"
#include <Timer.h>
#include <Camera.h>
#include <InputManager.h>
#include <iostream>

void Game::Init(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool)
{
	Camera::GetInstance().Init(&InputManager::GetInstance(), { 0, 0, 5 });

	m_pScene3D = std::make_unique<Scene>(device, physicalDevice, commandPool);
	

	for (int x{}; x < 4; ++x)
	{
		for (int z{}; z < 4; ++z)
		{
			m_Chunks.emplace_back(std::make_unique<Chunk>(glm::vec3{ x*16 + x,0,z*16 + z }, 16, 128, 16, device, physicalDevice, commandPool));

		}
	}
	//m_Chunks.emplace_back(std::make_unique<Chunk>(glm::vec3{ 0,0,0 }, 16, 256, 16, device, physicalDevice, commandPool));
	//m_Chunks.emplace_back(std::make_unique<Chunk>(glm::vec3{ 16,0,0 }, 16, 256, 16, device, physicalDevice, commandPool));
	//m_Chunks.emplace_back(std::make_unique<Chunk>(glm::vec3{ 32,0,0 }, 16, 256, 16, device, physicalDevice, commandPool));
	//m_Chunks.emplace_back(std::make_unique<Chunk>(glm::vec3{ 0,0,16 }, 16, 256, 16, device, physicalDevice, commandPool));
	//m_Chunks.emplace_back(std::make_unique<Chunk>(glm::vec3{ 0,0,32 }, 16, 256, 16, device, physicalDevice, commandP;
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
	//m_Chunk->Render(commandBuffer, pipelineLayout);
	for (const auto& chunk : m_Chunks)
	{
		chunk->Render(commandBuffer, pipelineLayout);
	}
}

void Game::Render2D(VkCommandBuffer commandBuffer)
{
	m_pScene2D->Render(commandBuffer);
}

void Game::Destroy(VkDevice device)
{
	m_pScene2D->CleanUp();
	m_pScene3D->CleanUp();
	for (auto& chunk : m_Chunks)
	{
		chunk->Destroy(device);
	}
	//m_Chunk->Destroy(device);
	//m_pTextureAtlas->Destroy(device);
	//for (auto& texture : m_pTextures)
	//{
	//	texture.Destroy(device);
	//}
}
