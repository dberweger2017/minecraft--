#pragma once

#include "Chunk.hpp"
#include "Vertex.hpp"
#include <vector>

namespace ChunkMesher {
    // Generates a list of vertices for a single chunk using Face Culling.
    // cx, cy are the chunk coordinates used to offset the vertex positions.
    std::vector<Vertex> generateMesh(const Chunk& chunk, int cx, int cy);
}
