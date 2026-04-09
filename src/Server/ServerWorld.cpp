#include "ServerWorld.hpp"
#include "Core/Logger.hpp"
#include "Core/FastNoiseLite.h"
#include <chrono>

ServerWorld::ServerWorld(Network::ThreadSafeQueue<Network::ClientMessage>& incoming,
                         Network::ThreadSafeQueue<Network::ServerMessage>& outgoing)
    : incomingMessages_(incoming), outgoingMessages_(outgoing), seed_(1337) {
}

ServerWorld::~ServerWorld() {
    stop();
}

void ServerWorld::start() {
    if (!running_) {
        running_ = true;
        serverThread_ = std::thread(&ServerWorld::run, this);
        Logger::log("ServerWorld: Started thread");
    }
}

void ServerWorld::stop() {
    if (running_) {
        running_ = false;
        if (serverThread_.joinable()) {
            serverThread_.join();
            Logger::log("ServerWorld: Stopped thread");
        }
    }
}

void ServerWorld::run() {
    while (running_) {
        processMessages();
        
        // Sleep to yield CPU, standard tick rate (~50ms / 20 TPS)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void ServerWorld::processMessages() {
    while (auto msgOpt = incomingMessages_.try_pop()) {
        auto& msg = msgOpt.value();
        
        if (std::holds_alternative<Network::ChunkRequest>(msg)) {
            auto req = std::get<Network::ChunkRequest>(msg);
            handleChunkRequest(req.cx, req.cy);
        } else if (std::holds_alternative<Network::BlockEditRequest>(msg)) {
            auto req = std::get<Network::BlockEditRequest>(msg);
            handleBlockEdit(req.x, req.y, req.z, req.type);
        }
    }
}

void ServerWorld::handleChunkRequest(int cx, int cy) {
    if (chunks_.find({cx, cy}) == chunks_.end()) {
        auto newChunk = std::make_unique<Chunk>();
        generateTerrain(newChunk.get(), cx, cy);
        
        // Deep copy the chunk to send to the client (so server keeps its authority)
        auto snapshot = std::make_unique<Chunk>();
        std::memcpy(snapshot->blocks, newChunk->blocks, sizeof(Chunk::blocks));
        
        chunks_[{cx, cy}] = std::move(newChunk);
        
        outgoingMessages_.push(Network::ChunkSnapshot{cx, cy, std::move(snapshot)});
        Logger::log("ServerWorld: Generated and sent chunk (" + std::to_string(cx) + ", " + std::to_string(cy) + ")");
    } else {
        // Send a snapshot of the existing chunk
        auto snapshot = std::make_unique<Chunk>();
        std::memcpy(snapshot->blocks, chunks_[{cx, cy}]->blocks, sizeof(Chunk::blocks));
        outgoingMessages_.push(Network::ChunkSnapshot{cx, cy, std::move(snapshot)});
    }
}

void ServerWorld::handleBlockEdit(int x, int y, int z, BlockType type) {
    int cx = std::floor((float)x / CHUNK_WIDTH);
    int cy = std::floor((float)y / CHUNK_WIDTH);
    int bx = x - cx * CHUNK_WIDTH;
    int by = y - cy * CHUNK_WIDTH;

    auto it = chunks_.find({cx, cy});
    if (it != chunks_.end()) {
        it->second->setBlock(bx, by, z, type);
        // Broadcast block update
        outgoingMessages_.push(Network::BlockUpdate{x, y, z, type});
        Logger::log("ServerWorld: Processed Block Edit at (" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")");
    }
}

void ServerWorld::generateTerrain(Chunk* chunk, int cx, int cy) {
    // 1. Continentalness (Base height bias, very low frequency)
    FastNoiseLite continentalNoise;
    continentalNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    continentalNoise.SetSeed(seed_);
    continentalNoise.SetFrequency(0.001f); // Much wider landmasses
    continentalNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    continentalNoise.SetFractalOctaves(4);

    // 2. Erosion (Mountain/Valley shapes, low frequency)
    FastNoiseLite erosionNoise;
    erosionNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    erosionNoise.SetSeed(seed_ + 1);
    erosionNoise.SetFrequency(0.003f); // Much wider mountains
    erosionNoise.SetFractalType(FastNoiseLite::FractalType_Ridged);
    erosionNoise.SetFractalOctaves(4);

    // 3. 3D Density Noise (Overhangs, arches, volumetric detail)
    FastNoiseLite densityNoise3D;
    densityNoise3D.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    densityNoise3D.SetSeed(seed_ + 2);
    densityNoise3D.SetFrequency(0.015f); // Chunkier overhangs
    densityNoise3D.SetFractalType(FastNoiseLite::FractalType_FBm);
    densityNoise3D.SetFractalOctaves(3);

    // 4. 3D Cave Noise (Spaghetti caves/tunnels)
    FastNoiseLite caveNoise3D;
    caveNoise3D.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    caveNoise3D.SetSeed(seed_ + 3);
    caveNoise3D.SetFrequency(0.02f); // Wider caves
    caveNoise3D.SetFractalType(FastNoiseLite::FractalType_Ridged);
    caveNoise3D.SetFractalOctaves(2);

    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int y = 0; y < CHUNK_WIDTH; ++y) {
            float worldX = static_cast<float>(cx * CHUNK_WIDTH + x);
            float worldY = static_cast<float>(cy * CHUNK_WIDTH + y);

            // Step 1: Base Height calculation from 2D noises
            float cVal = continentalNoise.GetNoise(worldX, worldY); // [-1, 1]
            float eVal = erosionNoise.GetNoise(worldX, worldY);     // [-1, 1]

            // Transform cVal and eVal into a base height
            // Scaled up amplitude for much taller features
            float baseHeight = -250.0f + (cVal * 150.0f) + (eVal * 100.0f);

            int topSolidZ = -CHUNK_HEIGHT - 1; // Track highest solid block for surface rules
            bool solid[CHUNK_HEIGHT] = {false};

            // Step 2: 3D Density Field Evaluation & Cave Carving
            for (int z = 0; z > -CHUNK_HEIGHT; --z) {
                float worldZ = static_cast<float>(z);

                // Initial density based on height gradient
                // Positive if below baseHeight (solid rock), negative if above (air)
                float gradient = baseHeight - worldZ; 
                
                // Add 3D noise to the gradient (scale determines how "chunky" it is)
                float d3D = densityNoise3D.GetNoise(worldX, worldY, worldZ) * 15.0f;
                float finalDensity = gradient + d3D;

                if (finalDensity > 0.0f) {
                    // We are in solid mass, now check carvers (Caves)
                    float c3D = caveNoise3D.GetNoise(worldX, worldY, worldZ);
                    
                    // A "worm" tunnel is where the noise crosses 0. 
                    // If the absolute value is very small, we are inside the tunnel.
                    // We also ensure caves only form underground, not floating in the sky.
                    bool isCave = (std::abs(c3D) < 0.1f) && (worldZ < -10.0f);
                    
                    if (!isCave) {
                        solid[-z] = true; // -z is our index from 0 to 511
                        if (topSolidZ < z) {
                            topSolidZ = z;
                        }
                    }
                }
            }

            // Step 3: Surface Rules Pass
            for (int z = 0; z > -CHUNK_HEIGHT; --z) {
                int zIdx = -z;
                BlockType type = BlockType::Air;

                if (z == -511) {
                    type = BlockType::Bedrock;
                } else if (solid[zIdx]) {
                    // How deep are we from the very top of the terrain?
                    // This correctly places grass on the top of overhangs!
                    int depth = topSolidZ - z;
                    
                    if (depth == 0) {
                        type = (z <= -120) ? BlockType::Dirt : BlockType::Grass;
                    } else if (depth < 4) {
                        type = BlockType::Dirt;
                    } else {
                        type = BlockType::Stone;
                    }
                } else if (z <= -120) {
                    type = BlockType::Water;
                }

                chunk->setBlock(x, y, z, type);
            }
        }
    }
}
