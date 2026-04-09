#include "ChunkMesher.hpp"
#include "World.hpp"
#include <glm/glm.hpp>

namespace ChunkMesher {

static void addCulledCubeToMesh(const float x, const float y, const float z, const BlockType type, const bool neighbors[6], std::vector<Vertex>& vertices) {
    glm::vec4 color = glm::vec4(1.0f);
    if (type == BlockType::Grass) {
        color = glm::vec4(0.13f, 0.55f, 0.13f, 1.0f);
    } else if (type == BlockType::Dirt) {
        color = glm::vec4(0.55f, 0.27f, 0.07f, 1.0f);
    } else if (type == BlockType::Stone) {
        color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    } else if (type == BlockType::Bedrock) {
        color = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
    } else if (type == BlockType::Water) {
        color = glm::vec4(0.2f, 0.4f, 0.8f, 0.5f);
    }

    const float x0 = x - 0.5f, x1 = x + 0.5f;
    const float y0 = y - 0.5f, y1 = y + 0.5f;
    const float z0 = z - 0.5f, z1 = z + 0.5f;

    // UVs for a face: (0,0), (1,0), (1,1), (0,1)
    glm::vec2 uv00 = {0, 0}, uv10 = {1, 0}, uv11 = {1, 1}, uv01 = {0, 1};

    // Back (-Y)
    if (!neighbors[2]) {
        glm::vec3 n = {0, -1, 0};
        vertices.push_back({{x0, y0, z0}, color, n, uv00}); vertices.push_back({{x1, y0, z0}, color, n, uv10}); vertices.push_back({{x1, y0, z1}, color, n, uv11});
        vertices.push_back({{x1, y0, z1}, color, n, uv11}); vertices.push_back({{x0, y0, z1}, color, n, uv01}); vertices.push_back({{x0, y0, z0}, color, n, uv00});
    }
    // Front (+Y)
    if (!neighbors[3]) {
        glm::vec3 n = {0, 1, 0};
        vertices.push_back({{x0, y1, z0}, color, n, uv00}); vertices.push_back({{x0, y1, z1}, color, n, uv01}); vertices.push_back({{x1, y1, z1}, color, n, uv11});
        vertices.push_back({{x1, y1, z1}, color, n, uv11}); vertices.push_back({{x1, y1, z0}, color, n, uv10}); vertices.push_back({{x0, y1, z0}, color, n, uv00});
    }
    // Left (-X)
    if (!neighbors[0]) {
        glm::vec3 n = {-1, 0, 0};
        vertices.push_back({{x0, y0, z0}, color, n, uv00}); vertices.push_back({{x0, y0, z1}, color, n, uv01}); vertices.push_back({{x0, y1, z1}, color, n, uv11});
        vertices.push_back({{x0, y1, z1}, color, n, uv11}); vertices.push_back({{x0, y1, z0}, color, n, uv10}); vertices.push_back({{x0, y0, z0}, color, n, uv00});
    }
    // Right (+X)
    if (!neighbors[1]) {
        glm::vec3 n = {1, 0, 0};
        vertices.push_back({{x1, y0, z0}, color, n, uv00}); vertices.push_back({{x1, y1, z0}, color, n, uv10}); vertices.push_back({{x1, y1, z1}, color, n, uv11});
        vertices.push_back({{x1, y1, z1}, color, n, uv11}); vertices.push_back({{x1, y0, z1}, color, n, uv01}); vertices.push_back({{x1, y0, z0}, color, n, uv00});
    }
    // Bottom (-Z)
    if (!neighbors[4]) {
        glm::vec3 n = {0, 0, -1};
        vertices.push_back({{x0, y0, z0}, color, n, uv00}); vertices.push_back({{x0, y1, z0}, color, n, uv01}); vertices.push_back({{x1, y1, z0}, color, n, uv11});
        vertices.push_back({{x1, y1, z0}, color, n, uv11}); vertices.push_back({{x1, y0, z0}, color, n, uv10}); vertices.push_back({{x0, y0, z0}, color, n, uv00});
    }
    // Top (+Z)
    if (!neighbors[5]) {
        glm::vec3 n = {0, 0, 1};
        vertices.push_back({{x0, y0, z1}, color, n, uv00}); vertices.push_back({{x1, y0, z1}, color, n, uv10}); vertices.push_back({{x1, y1, z1}, color, n, uv11});
        vertices.push_back({{x1, y1, z1}, color, n, uv11}); vertices.push_back({{x0, y1, z1}, color, n, uv01}); vertices.push_back({{x0, y0, z1}, color, n, uv00});
    }
}

std::vector<Vertex> generateMesh(const Chunk& chunk, int cx, int cy, World* world) {
    std::vector<Vertex> vertices;

    // Fetch neighbor chunks once to avoid mutex lock contention
    Chunk* chunkMinusX = world->getChunk(cx - 1, cy);
    Chunk* chunkPlusX  = world->getChunk(cx + 1, cy);
    Chunk* chunkMinusY = world->getChunk(cx, cy - 1);
    Chunk* chunkPlusY  = world->getChunk(cx, cy + 1);

    auto shouldCull = [](BlockType current, BlockType neighbor) {
        if (neighbor != BlockType::Air && neighbor != BlockType::Water) return true;
        if (current == BlockType::Water && neighbor == BlockType::Water) return true;
        return false;
    };

    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int y = 0; y < CHUNK_WIDTH; ++y) {
            for (int z = 0; z > -CHUNK_HEIGHT; --z) {
                const Block block = chunk.getBlock(x, y, z);
                if (block.type != BlockType::Air) {
                    bool neighbors[6];
                    
                    // -X
                    BlockType n0 = (x > 0) ? chunk.getBlock(x - 1, y, z).type : (chunkMinusX ? chunkMinusX->getBlock(CHUNK_WIDTH - 1, y, z).type : BlockType::Stone);
                    neighbors[0] = shouldCull(block.type, n0);
                    
                    // +X
                    BlockType n1 = (x < CHUNK_WIDTH - 1) ? chunk.getBlock(x + 1, y, z).type : (chunkPlusX ? chunkPlusX->getBlock(0, y, z).type : BlockType::Stone);
                    neighbors[1] = shouldCull(block.type, n1);
                    
                    // -Y
                    BlockType n2 = (y > 0) ? chunk.getBlock(x, y - 1, z).type : (chunkMinusY ? chunkMinusY->getBlock(x, CHUNK_WIDTH - 1, z).type : BlockType::Stone);
                    neighbors[2] = shouldCull(block.type, n2);
                    
                    // +Y
                    BlockType n3 = (y < CHUNK_WIDTH - 1) ? chunk.getBlock(x, y + 1, z).type : (chunkPlusY ? chunkPlusY->getBlock(x, 0, z).type : BlockType::Stone);
                    neighbors[3] = shouldCull(block.type, n3);
                    
                    // -Z (Bottom)
                    BlockType n4 = (z > -CHUNK_HEIGHT + 1) ? chunk.getBlock(x, y, z - 1).type : BlockType::Air;
                    neighbors[4] = shouldCull(block.type, n4);
                    
                    // +Z (Top)
                    BlockType n5 = (z < 0) ? chunk.getBlock(x, y, z + 1).type : BlockType::Air;
                    neighbors[5] = shouldCull(block.type, n5);

                    int worldX = cx * CHUNK_WIDTH + x;
                    int worldY = cy * CHUNK_WIDTH + y;

                    addCulledCubeToMesh(
                        static_cast<float>(worldX),
                        static_cast<float>(worldY),
                        static_cast<float>(z),
                        block.type, neighbors, vertices);
                }
            }
        }
    }
    return vertices;
}

} // namespace ChunkMesher
