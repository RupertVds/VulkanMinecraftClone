#include "Chunk.h"
#include <ChunkGenerator.h>

void Chunk::Update()
{
    if (m_IsMarkedForDeletion)
    {
        const float deltaTime = Timer::GetInstance().GetElapsed();

        // Increment deletion timer
        m_DeletionTimer += deltaTime;

        // If time for deletion destroy chunk
        if (m_DeletionTimer >= ChunkGenerator::GetInstance().GetChunkDeletionTime())
        {
            // do it with a boolean so the chunkgenerator can check and delete it properly while being notified
            m_IsDeleted = true;
        }
    }
}
