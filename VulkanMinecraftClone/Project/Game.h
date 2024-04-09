#pragma once
#include "Scene2D.h"
#include "Scene.h"
#include <memory>

class Game final
{
public:
	Game() = default;
	~Game() = default;

	Game(const Game& other) = delete;
	Game& operator=(const Game& other) = delete;
	Game(Game&& other) = delete;
	Game& operator=(Game&& other) = delete;
public:
	void Init(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool);
	void Update();
	void Render(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
	void Render2D(VkCommandBuffer commandBuffer);
	void Destroy();
private:
	std::unique_ptr<Scene2D> m_pScene2D{};
	std::unique_ptr<Scene> m_pScene3D{};
	float m_PrintTimer{};
	const float m_PrintDelay{ 1.f };
};
