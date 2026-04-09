#pragma once

#include <map>
#include <memory>
#include <thread>
#include <atomic>
#include "World/Chunk.hpp"
#include "Network/Messages.hpp"
#include "Network/ThreadSafeQueue.hpp"

class ServerWorld {
public:
    ServerWorld(Network::ThreadSafeQueue<Network::ClientMessage>& incoming,
                Network::ThreadSafeQueue<Network::ServerMessage>& outgoing);
    ~ServerWorld();

    void start();
    void stop();

private:
    void run();
    void processMessages();
    void handleChunkRequest(int cx, int cy);
    void handleBlockEdit(int x, int y, int z, BlockType type);

    void generateTerrain(Chunk* chunk, int cx, int cy);

    Network::ThreadSafeQueue<Network::ClientMessage>& incomingMessages_;
    Network::ThreadSafeQueue<Network::ServerMessage>& outgoingMessages_;

    std::map<std::pair<int, int>, std::unique_ptr<Chunk>> chunks_;

    std::thread serverThread_;
    std::atomic<bool> running_{false};

    // World Seed
    int seed_;
};
