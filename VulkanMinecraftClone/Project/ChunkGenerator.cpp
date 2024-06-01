#include "ChunkGenerator.h"

const int ChunkGenerator::m_ViewDistance{ 10 };  // View distance in grid tiles
const int ChunkGenerator::m_LoadDistance{ 2 }; // Load distance in grid tiles
const int ChunkGenerator::m_Padding{ 2 }; // Padding for chunk loading
const int ChunkGenerator::m_NoiseFractals{ 8 }; // Amount of fractals for the noise
const float ChunkGenerator::m_ChunkDeletionTime{ 10.f }; // Time to delete chunks after being marked for deletion

void ChunkGenerator::Init(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool)
{
    this->m_Device = device;
    this->m_PhysicalDevice = physicalDevice;
    this->m_CommandPool = commandPool;
    LoadBlockData("textures/blockdata.json");

    // frequency, amplitude, lacunarity, persistence
    const float frequency = 0.005f;
    const float amplitude = 1.f;
    const float lacunarity = 2.f;
    const float persistence = 1/lacunarity;

    m_pSimplexNoise = std::make_unique<SimplexNoise>(frequency, amplitude, lacunarity, persistence);
    //m_pSimplexNoise = std::make_unique<SimplexNoise>(0.005f, 10.f, 2.f, 15.f);

    // Initialize the player's chunk position
    m_PlayerChunkPosition = CalculateChunkPosition(Camera::GetInstance().m_Position);

    // Initialize chunks around the player
    UpdateChunksAroundPlayer();
}


// fractals, frequency, amplitude, lacunarity, persistence
// 8, 0.005f, 1.f, 2.f, 0.25f
//const float frequency = 0.005f;
//const float amplitude = 1.f;
//const float lacunarity = 2.f;
//const float persistence = 1 / lacunarity;