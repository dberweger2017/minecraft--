#pragma once

#include "Chunk.hpp"
#include "Vertex.hpp"
#include <map>
#include <vector>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <deque>
#include <set>

struct ChunkMesh {
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexMemory = VK_NULL_HANDLE;
    uint32_t vertexCount = 0;
};

struct ChunkBuildResult {
    int x, y;
    std::vector<Vertex> vertices;
};

class World {
public:
    std::map<std::pair<int, int>, std::unique_ptr<Chunk>> chunks;
    mutable std::shared_mutex chunkMutex;

    std::map<std::pair<int, int>, ChunkMesh> meshes;
    mutable std::mutex meshMutex;

    // Async Queues
    std::deque<std::pair<int, int>> loadQueue;
    std::deque<ChunkBuildResult> buildResults;
    std::set<std::pair<int, int>> pendingChunks; // Track what's already being loaded
    std::mutex queueMutex;

    glm::vec3 sunPos = glm::vec3(0.0f);
    glm::vec3 moonPos = glm::vec3(0.0f);
    ChunkMesh sunMesh;
    ChunkMesh moonMesh;

    void addChunk(int x, int y) {
        {
            std::shared_lock<std::shared_mutex> lock(chunkMutex);
            if (chunks.find({x, y}) != chunks.end()) return;
        }
        
        auto newChunk = std::make_unique<Chunk>();
        generateTerrain(newChunk.get(), x, y);
        
        std::unique_lock<std::shared_mutex> lock(chunkMutex);
        chunks[{x, y}] = std::move(newChunk);
    }

    Chunk* getChunk(int x, int y) {
        std::shared_lock<std::shared_mutex> lock(chunkMutex);
        auto it = chunks.find({x, y});
        if (it != chunks.end()) return it->second.get();
        return nullptr;
    }

    bool isBlockSolid(int x, int y, int z) {
        int cx = std::floor((float)x / CHUNK_WIDTH);
        int cy = std::floor((float)y / CHUNK_WIDTH);
        int bx = x - cx * CHUNK_WIDTH;
        int by = y - cy * CHUNK_WIDTH;

        std::shared_lock<std::shared_mutex> lock(chunkMutex);
        auto it = chunks.find({cx, cy});
        if (it == chunks.end()) return false; // Air for missing chunks
        return it->second->getBlock(bx, by, z).isSolid();
    }

private:
    void generateTerrain(Chunk* chunk, int cx, int cy) {
        for (int x = 0; x < CHUNK_WIDTH; ++x) {
            for (int y = 0; y < CHUNK_WIDTH; ++y) {
                for (int z = 0; z > -CHUNK_HEIGHT; --z) {
                    BlockType type = BlockType::Air;
                    if (z == 0) type = BlockType::Grass;
                    else if (z > -4) type = BlockType::Dirt;
                    else if (z > -511) type = BlockType::Stone;
                    else if (z == -511) type = BlockType::Bedrock;
                    chunk->setBlock(x, y, z, type);
                }
            }
        }
    }
};
