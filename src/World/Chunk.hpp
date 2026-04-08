#pragma once

#include "Block.hpp"

constexpr int CHUNK_SIZE = 16;

struct Chunk {
  Block blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];

  void setBlock(const int x, const int y, const int z, const BlockType type) {
    if (x >= 0 && x < CHUNK_SIZE && y >= 0 && y < CHUNK_SIZE && z >= 0 && z < CHUNK_SIZE) {
      blocks[x][y][z].type = type;
    }
  }

  [[nodiscard]] Block getBlock(const int x, const int y, const int z) const {
    if (x >= 0 && x < CHUNK_SIZE && y >= 0 && y < CHUNK_SIZE && z >= 0 && z < CHUNK_SIZE) {
      return blocks[x][y][z];
    }
    return {BlockType::Air};
  }
};
