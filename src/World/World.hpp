#pragma once

#include "Chunk.hpp"
#include "Vertex.hpp"
#include <map>
#include <vector>
#include <memory>
#include <mutex>

struct ChunkMesh {
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexMemory = VK_NULL_HANDLE;
    uint32_t vertexCount = 0;
};

class World {
public:
    std::map<std::pair<int, int>, std::unique_ptr<Chunk>> chunks;
    std::map<std::pair<int, int>, ChunkMesh> meshes;
    mutable std::mutex meshMutex;

    void addChunk(int x, int y) {
        if (chunks.find({x, y}) == chunks.end()) {
            chunks[{x, y}] = std::make_unique<Chunk>();
            generateTerrain(x, y);
        }
    }

    Chunk* getChunk(int x, int y) {
        auto it = chunks.find({x, y});
        if (it != chunks.end()) return it->second.get();
        return nullptr;
    }

    bool isBlockSolid(int x, int y, int z) {
        int cx = std::floor((float)x / CHUNK_WIDTH);
        int cy = std::floor((float)y / CHUNK_WIDTH);
        int bx = x - cx * CHUNK_WIDTH;
        int by = y - cy * CHUNK_WIDTH;

        Chunk* chunk = getChunk(cx, cy);
        if (!chunk) return false; // Air for missing chunks
        return chunk->getBlock(bx, by, z).isSolid();
    }

private:
    void generateTerrain(int cx, int cy) {
        Chunk* chunk = chunks[{cx, cy}].get();
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
