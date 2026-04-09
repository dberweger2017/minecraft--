#pragma once

#include <cstdint>

enum class BlockType : uint8_t {
  Air = 0,
  Dirt = 1,
  Grass = 2,
  Stone = 3,
  Wood = 4,
  Leaves = 5,
  Bedrock = 6,
  Water = 7
};

struct Block {
  BlockType type = BlockType::Air;

  [[nodiscard]] bool isSolid() const {
    return type != BlockType::Air;
  }

  [[nodiscard]] bool isOpaque() const {
    return type != BlockType::Air && type != BlockType::Water;
  }
};
