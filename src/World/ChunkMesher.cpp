#include "ChunkMesher.hpp"
#include "World.hpp"
#include <glm/glm.hpp>

namespace ChunkMesher {

static void addCulledCubeToMesh(const float x, const float y, const float z, const BlockType type, const bool neighbors[6], std::vector<Vertex>& vertices) {
    glm::vec3 color = glm::vec3(1.0f);
    if (type == BlockType::Grass) {
        color = glm::vec3(0.13f, 0.55f, 0.13f);
    } else if (type == BlockType::Dirt) {
        color = glm::vec3(0.55f, 0.27f, 0.07f);
    } else if (type == BlockType::Stone) {
        color = glm::vec3(0.5f, 0.5f, 0.5f);
    } else if (type == BlockType::Bedrock) {
        color = glm::vec3(0.1f, 0.1f, 0.1f);
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

    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int y = 0; y < CHUNK_WIDTH; ++y) {
            for (int z = 0; z > -CHUNK_HEIGHT; --z) {
                const Block block = chunk.getBlock(x, y, z);
                if (block.isSolid()) {
                    int worldX = cx * CHUNK_WIDTH + x;
                    int worldY = cy * CHUNK_WIDTH + y;

                    bool neighbors[6];
                    neighbors[0] = world->isBlockSolid(worldX - 1, worldY, z);
                    neighbors[1] = world->isBlockSolid(worldX + 1, worldY, z);
                    neighbors[2] = world->isBlockSolid(worldX, worldY - 1, z);
                    neighbors[3] = world->isBlockSolid(worldX, worldY + 1, z);
                    neighbors[4] = world->isBlockSolid(worldX, worldY, z - 1);
                    neighbors[5] = world->isBlockSolid(worldX, worldY, z + 1);

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
