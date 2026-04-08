#include "ChunkMesher.hpp"
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

    // Neighbors order: 0:-X, 1:+X, 2:-Y, 3:+Y, 4:-Z, 5:+Z
    
    // Back (-Y)
    if (!neighbors[2]) {
        glm::vec3 n = {0, -1, 0};
        vertices.push_back({{x0, y0, z0}, color, n}); vertices.push_back({{x1, y0, z0}, color, n}); vertices.push_back({{x1, y0, z1}, color, n});
        vertices.push_back({{x1, y0, z1}, color, n}); vertices.push_back({{x0, y0, z1}, color, n}); vertices.push_back({{x0, y0, z0}, color, n});
    }
    // Front (+Y)
    if (!neighbors[3]) {
        glm::vec3 n = {0, 1, 0};
        vertices.push_back({{x0, y1, z0}, color, n}); vertices.push_back({{x0, y1, z1}, color, n}); vertices.push_back({{x1, y1, z1}, color, n});
        vertices.push_back({{x1, y1, z1}, color, n}); vertices.push_back({{x1, y1, z0}, color, n}); vertices.push_back({{x0, y1, z0}, color, n});
    }
    // Left (-X)
    if (!neighbors[0]) {
        glm::vec3 n = {-1, 0, 0};
        vertices.push_back({{x0, y0, z0}, color, n}); vertices.push_back({{x0, y0, z1}, color, n}); vertices.push_back({{x0, y1, z1}, color, n});
        vertices.push_back({{x0, y1, z1}, color, n}); vertices.push_back({{x0, y1, z0}, color, n}); vertices.push_back({{x0, y0, z0}, color, n});
    }
    // Right (+X)
    if (!neighbors[1]) {
        glm::vec3 n = {1, 0, 0};
        vertices.push_back({{x1, y0, z0}, color, n}); vertices.push_back({{x1, y1, z0}, color, n}); vertices.push_back({{x1, y1, z1}, color, n});
        vertices.push_back({{x1, y1, z1}, color, n}); vertices.push_back({{x1, y0, z1}, color, n}); vertices.push_back({{x1, y0, z0}, color, n});
    }
    // Bottom (-Z)
    if (!neighbors[4]) {
        glm::vec3 n = {0, 0, -1};
        vertices.push_back({{x0, y0, z0}, color, n}); vertices.push_back({{x0, y1, z0}, color, n}); vertices.push_back({{x1, y1, z0}, color, n});
        vertices.push_back({{x1, y1, z0}, color, n}); vertices.push_back({{x1, y0, z0}, color, n}); vertices.push_back({{x0, y0, z0}, color, n});
    }
    // Top (+Z)
    if (!neighbors[5]) {
        glm::vec3 n = {0, 0, 1};
        vertices.push_back({{x0, y0, z1}, color, n}); vertices.push_back({{x1, y0, z1}, color, n}); vertices.push_back({{x1, y1, z1}, color, n});
        vertices.push_back({{x1, y1, z1}, color, n}); vertices.push_back({{x0, y1, z1}, color, n}); vertices.push_back({{x0, y0, z1}, color, n});
    }
}

std::vector<Vertex> generateMesh(const Chunk& chunk, int cx, int cy) {
    std::vector<Vertex> vertices;

    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int y = 0; y < CHUNK_WIDTH; ++y) {
            for (int z = 0; z > -CHUNK_HEIGHT; --z) {
                const Block block = chunk.getBlock(x, y, z);
                if (block.isSolid()) {
                    bool neighbors[6];
                    neighbors[0] = chunk.getBlock(x - 1, y, z).isSolid();
                    neighbors[1] = chunk.getBlock(x + 1, y, z).isSolid();
                    neighbors[2] = chunk.getBlock(x, y - 1, z).isSolid();
                    neighbors[3] = chunk.getBlock(x, y + 1, z).isSolid();
                    neighbors[4] = chunk.getBlock(x, y, z - 1).isSolid();
                    neighbors[5] = chunk.getBlock(x, y, z + 1).isSolid();

                    addCulledCubeToMesh(
                        static_cast<float>(cx * CHUNK_WIDTH + x),
                        static_cast<float>(cy * CHUNK_WIDTH + y),
                        static_cast<float>(z),
                        block.type, neighbors, vertices);
                }
            }
        }
    }
    return vertices;
}

} // namespace ChunkMesher
