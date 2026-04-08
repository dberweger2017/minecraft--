#pragma once

#include "Block.hpp"

constexpr int CHUNK_WIDTH = 16;
constexpr int CHUNK_HEIGHT = 512;

struct Chunk {
  // We use a 1D array or adjusted indexing to handle the height
  // For simplicity in this stage, we'll use [width][width][height]
  Block blocks[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_HEIGHT];

  void setBlock(const int x, const int y, const int z, const BlockType type) {
    // Map Z from 0 to -511 into array index 511 down to 0
    int zIdx = z + (CHUNK_HEIGHT - 1); 
    if (x >= 0 && x < CHUNK_WIDTH && y >= 0 && y < CHUNK_WIDTH && zIdx >= 0 && zIdx < CHUNK_HEIGHT) {
      blocks[x][y][zIdx].type = type;
    }
  }

  [[nodiscard]] Block getBlock(const int x, const int y, const int z) const {
    int zIdx = z + (CHUNK_HEIGHT - 1);
    if (x >= 0 && x < CHUNK_WIDTH && y >= 0 && y < CHUNK_WIDTH && zIdx >= 0 && zIdx < CHUNK_HEIGHT) {
      return blocks[x][y][zIdx];
    }
    return {BlockType::Air};
  }
};
