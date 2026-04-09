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

    void receiveSnapshot(int x, int y, std::unique_ptr<Chunk> chunkData) {
        std::unique_lock<std::shared_mutex> lock(chunkMutex);
        chunks[{x, y}] = std::move(chunkData);
    }

    Chunk* getChunk(int x, int y) {
        std::shared_lock<std::shared_mutex> lock(chunkMutex);
        auto it = chunks.find({x, y});
        if (it != chunks.end()) return it->second.get();
        return nullptr;
    }

    BlockType getBlockType(int x, int y, int z) {
        int cx = std::floor((float)x / CHUNK_WIDTH);
        int cy = std::floor((float)y / CHUNK_WIDTH);
        int bx = x - cx * CHUNK_WIDTH;
        int by = y - cy * CHUNK_WIDTH;

        std::shared_lock<std::shared_mutex> lock(chunkMutex);
        auto it = chunks.find({cx, cy});
        if (it == chunks.end()) return BlockType::Air; // Air for missing chunks
        return it->second->getBlock(bx, by, z).type;
    }

private:
};
