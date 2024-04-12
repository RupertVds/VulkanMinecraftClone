#pragma once
#include "Scene2D.h"
#include "Scene.h"
#include <memory>
#include "Texture.h"
#include <Chunk.h>

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
	void Destroy(VkDevice device);

	//const std::vector<Texture>& GetTextures() const { return m_pTextures; }
private:
	std::unique_ptr<Scene2D> m_pScene2D{};
	std::unique_ptr<Scene> m_pScene3D{};
	std::unique_ptr<Chunk> m_Chunk{};
	//std::vector<Texture> m_pTextures{};
	float m_PrintTimer{};
	const float m_PrintDelay{ 1.f };
};
