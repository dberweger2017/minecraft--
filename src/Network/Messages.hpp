#pragma once

#include <memory>
#include <variant>
#include "World/Chunk.hpp"

namespace Network {

// Client -> Server messages
struct ChunkRequest {
    int cx, cy;
};

struct BlockEditRequest {
    int x, y, z;
    BlockType type;
};

// A variant representing any message sent from Client to Server
using ClientMessage = std::variant<ChunkRequest, BlockEditRequest>;

// Server -> Client messages
struct ChunkSnapshot {
    int cx, cy;
    std::unique_ptr<Chunk> chunkData;
};

struct BlockUpdate {
    int x, y, z;
    BlockType type;
};

// A variant representing any message sent from Server to Client
using ServerMessage = std::variant<ChunkSnapshot, BlockUpdate>;

} // namespace Network
