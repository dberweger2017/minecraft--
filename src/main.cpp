#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <optional>
#include <chrono>
#include <cmath>
#include <thread>
#include <atomic>

#include "World/World.hpp"
#include "World/ChunkMesher.hpp"
#include "Server/ServerWorld.hpp"
#include "Camera/Camera.hpp"
#include "Renderer/Renderer.hpp"
#include "Core/Logger.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

enum class GameState {
    LOADING,
    INTRO_ANIMATION,
    PLAYING
};

class MinecraftApp {
public:
    Camera camera;
    void run() {
        initWindow();
        initApp();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window;
    std::unique_ptr<Renderer> renderer;
    World world;
    bool isPaused = false;
    bool showDebugScreen = false;
    GameState state = GameState::LOADING;
    float introTime = 0.0f;
    const float introDuration = 1.0f; // Fast drop
    float loadingProgress = 0.0f;

    std::vector<std::thread> workerThreads;
    std::atomic<bool> running{true};

    Network::ThreadSafeQueue<Network::ClientMessage> clientToServerQueue;
    Network::ThreadSafeQueue<Network::ServerMessage> serverToClientQueue;
    std::unique_ptr<ServerWorld> server;

    void initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(1280, 720, "Minecraft Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        
        // Input Callbacks
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        glfwSetCursorPosCallback(window, mouseCallback);
        glfwSetKeyCallback(window, keyCallback);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<MinecraftApp*>(glfwGetWindowUserPointer(window));
        if (app && app->renderer) {
            app->renderer->setFramebufferResized();
        }
    }

    void initApp() {
        RendererInitInfo initInfo{ window };
        renderer = std::make_unique<Renderer>(initInfo);
        
        initCelestialBodies();

        server = std::make_unique<ServerWorld>(clientToServerQueue, serverToClientQueue);
        server->start();

        // Initial setup for LOADING state
        camera.setPosition(glm::vec3(0.0f, 0.0f, 120.0f));
        camera.setRotation(-90.0f, -89.9f);

        // Start background workers (using half the CPU cores)
        unsigned int numThreads = std::thread::hardware_concurrency() / 2;
        if (numThreads == 0) numThreads = 1; // Fallback
        for (unsigned int i = 0; i < numThreads; ++i) {
            workerThreads.emplace_back(&MinecraftApp::workerLoop, this);
        }

        // Initial World Gen
        updateWorld(8);
        processCompletedMeshes(); 
    }

    void workerLoop() {
        while (running) {
            std::pair<int, int> coords;
            bool hasWork = false;

            {
                std::lock_guard<std::mutex> lock(world.queueMutex);
                if (!world.loadQueue.empty()) {
                    coords = world.loadQueue.front();
                    world.loadQueue.pop_front();
                    hasWork = true;
                }
            }

            if (hasWork) {
                // 2. Generate Mesh (CPU heavy)
                auto vertices = ChunkMesher::generateMesh(*world.getChunk(coords.first, coords.second), coords.first, coords.second, &world);
                
                Logger::log("Worker: Meshed chunk " + std::to_string(coords.first) + ", " + std::to_string(coords.second));

                // 3. Push to results
                {
                    std::lock_guard<std::mutex> lock(world.queueMutex);
                    world.buildResults.push_back({coords.first, coords.second, std::move(vertices)});
                }
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    }

    void initCelestialBodies() {
        const glm::vec3 positions[36] = {
            {-1,-1,-1}, {1,-1,-1}, {1,1,-1}, {1,1,-1}, {-1,1,-1}, {-1,-1,-1},
            {-1,-1, 1}, {1,-1, 1}, {1,1, 1}, {1,1, 1}, {-1,1, 1}, {-1,-1, 1},
            {-1, 1, 1}, {-1, 1,-1}, {-1,-1,-1}, {-1,-1,-1}, {-1,-1, 1}, {-1, 1, 1},
            { 1, 1, 1}, { 1, 1,-1}, { 1,-1,-1}, { 1,-1,-1}, { 1,-1, 1}, { 1, 1, 1},
            {-1,-1,-1}, { 1,-1,-1}, { 1,-1, 1}, { 1,-1, 1}, {-1,-1, 1}, {-1,-1,-1},
            {-1, 1,-1}, { 1, 1,-1}, { 1, 1, 1}, { 1, 1, 1}, {-1, 1, 1}, {-1, 1,-1}
        };

        std::vector<Vertex> sunVertices;
        glm::vec4 sunColor = {1.0f, 0.9f, 0.5f, 1.0f};
        for (const auto& p : positions) sunVertices.push_back({p * 20.0f, sunColor, {0,0,0}, {0,0}});
        world.sunMesh = renderer->createChunkMesh(sunVertices);

        std::vector<Vertex> moonVertices;
        glm::vec4 moonColor = {0.8f, 0.8f, 0.9f, 1.0f};
        for (const auto& p : positions) moonVertices.push_back({p * 15.0f, moonColor, {0,0,0}, {0,0}});
        world.moonMesh = renderer->createChunkMesh(moonVertices);
    }

    void processServerMessages() {
        while (auto msgOpt = serverToClientQueue.try_pop()) {
            auto& msg = msgOpt.value();
            if (std::holds_alternative<Network::ChunkSnapshot>(msg)) {
                auto& snapshot = std::get<Network::ChunkSnapshot>(msg);
                world.receiveSnapshot(snapshot.cx, snapshot.cy, std::move(snapshot.chunkData));
                
                std::lock_guard<std::mutex> lock(world.queueMutex);
                world.loadQueue.push_back({snapshot.cx, snapshot.cy});
            }
        }
    }

    void mainLoop() {
        auto lastTime = std::chrono::high_resolution_clock::now();
        float totalTime = 0.0f;

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            processServerMessages();

            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;
            totalTime += deltaTime;

            renderer->beginImGui();

            if (state == GameState::PLAYING) {
                if (!isPaused) {
                    camera.processInput(window, deltaTime, isPaused);
                    updateWorld();
                    processCompletedMeshes();
                }

                ImGui::SetNextWindowPos(ImVec2(10, 10));
                ImGui::SetNextWindowBgAlpha(0.3f);
                ImGui::Begin("Overlay", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove);
                ImGui::Text("X: %.1f Y: %.1f Z: %.1f", camera.pos.x, camera.pos.y, camera.pos.z);
                ImGui::Text("FPS: %.1f / 60.0", ImGui::GetIO().Framerate);
                ImGui::End();
            } else if (state == GameState::LOADING) {
                loadingProgress = updateWorld(8);
                processCompletedMeshes();
                drawLoadingUI();
                if (loadingProgress >= 1.0f) {
                    Logger::log("Loading Complete. Transitioning to Intro...");
                    state = GameState::INTRO_ANIMATION;
                    introTime = 0.0f;
                }
            } else if (state == GameState::INTRO_ANIMATION) {
                updateIntroAnimation(deltaTime);
                processCompletedMeshes();
            }

            // 20 minute day cycle
            constexpr float dayLength = 1200.0f;
            float dayProgress = fmod(totalTime, dayLength) / dayLength;
            float angle = dayProgress * 2.0f * 3.14159f;

            glm::vec3 sunDirection = glm::vec3(sin(angle), -cos(angle), 0.2f);
            float dist = 500.0f;
            world.sunPos = camera.pos + (sunDirection * -dist); 
            world.moonPos = camera.pos + (sunDirection * dist);

            glm::vec3 sunColor = glm::vec3(1.0f);
            glm::vec3 skyColor = glm::vec3(0.5f, 0.7f, 0.9f); 

            float sunHeight = -sunDirection.y; 
            if (sunHeight < 0.0f) {
                sunColor = glm::vec3(0.4f, 0.4f, 0.5f);
                skyColor = glm::vec3(0.02f, 0.02f, 0.05f);
            } else if (sunHeight < 0.4f) {
                float t = sunHeight / 0.4f;
                sunColor = glm::mix(glm::vec3(1.0f, 0.6f, 0.4f), glm::vec3(1.0f, 1.0f, 1.0f), t);
                skyColor = glm::mix(glm::vec3(0.2f, 0.1f, 0.1f), glm::vec3(0.5f, 0.7f, 0.9f), t);
            }

            if (showDebugScreen) {
                ImGui::SetNextWindowPos(ImVec2(10, 10));
                ImGui::Begin("Debug", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove);
                ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
                ImGui::Text("Pos: %.2f, %.2f, %.2f", camera.pos.x, camera.pos.y, camera.pos.z);
                ImGui::Text("Chunk: %d, %d", (int)std::floor(camera.pos.x / CHUNK_WIDTH), (int)std::floor(camera.pos.y / CHUNK_WIDTH));
                ImGui::End();
            }

            renderer->drawFrame(world, camera, sunDirection, sunColor, skyColor);
        }
        renderer->waitIdle();
    }

    void drawLoadingUI() {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("Loading", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);
        
        float windowWidth = ImGui::GetIO().DisplaySize.x;
        ImGui::SetCursorPos(ImVec2(windowWidth * 0.1f, 20));
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
        ImGui::ProgressBar(loadingProgress, ImVec2(windowWidth * 0.8f, 30), "");
        ImGui::PopStyleColor();

        const char* text = "GENERATING WORLD...";
        float textWidth = ImGui::CalcTextSize(text).x;
        ImGui::SetCursorPos(ImVec2((windowWidth - textWidth) * 0.5f, 60));
        ImGui::Text("%s", text);

        ImGui::End();
    }

    float spawnZ = 2.0f; // Default landing height

    void updateIntroAnimation(float deltaTime) {
        introTime += deltaTime;
        float t = introTime / introDuration;
        if (t >= 1.0f) {
            t = 1.0f;
            state = GameState::PLAYING;
            // Land at calculated height
            camera.setPosition(glm::vec3(8.0f, 8.0f, spawnZ));
            camera.setRotation(-90.0f, 0.0f);
            if (!isPaused) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            return;
        }

        float smoothT = t * t * (3.0f - 2.0f * t);

        float startZ = 120.0f;
        float startPitch = -89.9f;
        float endPitch = 0.0f;

        // Calculate landing height once during intro
        if (spawnZ == 2.0f) {
            Chunk* c = world.getChunk(0, 0);
            if (c) {
                for (int z = 0; z > -CHUNK_HEIGHT; --z) {
                    if (c->getBlock(8, 8, z).isSolid()) {
                        spawnZ = static_cast<float>(z) + 2.0f;
                        Logger::log("Intro: Calculated spawnZ = " + std::to_string(spawnZ) + " based on solid block at z=" + std::to_string(z));
                        break;
                    }
                }
            }
        }

        float curX = glm::mix(0.0f, 8.0f, smoothT);
        float curY = glm::mix(0.0f, 8.0f, smoothT);
        float curZ = glm::mix(startZ, spawnZ, smoothT);

        camera.setPosition(glm::vec3(curX, curY, curZ));
        camera.setRotation(-90.0f, glm::mix(startPitch, endPitch, smoothT));

        if (t >= 1.0f) {
            Logger::log("Intro: Finished landing at " + std::to_string(curX) + ", " + std::to_string(curY) + ", " + std::to_string(curZ));
        }
    }

    float updateWorld(int radius = 4) {
        const int playerChunkX = static_cast<int>(std::floor(camera.pos.x / CHUNK_WIDTH));
        const int playerChunkY = static_cast<int>(std::floor(camera.pos.y / CHUNK_WIDTH));

        int totalChunks = (radius * 2 + 1) * (radius * 2 + 1);
        int meshedChunks = 0;

        std::vector<std::pair<int, int>> newChunks;

        for (int x = playerChunkX - radius; x <= playerChunkX + radius; ++x) {
            for (int y = playerChunkY - radius; y <= playerChunkY + radius; ++y) {
                {
                    std::lock_guard<std::mutex> lock(world.meshMutex);
                    if (world.meshes.find({x, y}) != world.meshes.end()) {
                        meshedChunks++;
                        continue;
                    }
                }

                std::lock_guard<std::mutex> lock(world.queueMutex);
                if (world.pendingChunks.find({x, y}) == world.pendingChunks.end()) {
                    bool hasChunk = false;
                    {
                        std::shared_lock<std::shared_mutex> chunkLock(world.chunkMutex);
                        hasChunk = world.chunks.find({x, y}) != world.chunks.end();
                    }
                    if (!hasChunk) {
                        newChunks.push_back({x, y});
                    }
                }
            }
        }

        // Sort new chunks by distance to player
        std::sort(newChunks.begin(), newChunks.end(), [playerChunkX, playerChunkY](const auto& a, const auto& b) {
            int distA = (a.first - playerChunkX) * (a.first - playerChunkX) + (a.second - playerChunkY) * (a.second - playerChunkY);
            int distB = (b.first - playerChunkX) * (b.first - playerChunkX) + (b.second - playerChunkY) * (b.second - playerChunkY);
            return distA < distB; // Closest first
        });

        if (!newChunks.empty()) {
            std::lock_guard<std::mutex> lock(world.queueMutex);
            for (const auto& coords : newChunks) {
                clientToServerQueue.push(Network::ChunkRequest{coords.first, coords.second});
                world.pendingChunks.insert(coords);
            }
        }

        return (float)meshedChunks / totalChunks;
    }

    void processCompletedMeshes() {
        std::vector<ChunkBuildResult> results;
        {
            std::lock_guard<std::mutex> lock(world.queueMutex);
            while (!world.buildResults.empty()) {
                results.push_back(std::move(world.buildResults.front()));
                world.buildResults.pop_front();
            }
        }

        for (auto& res : results) {
            if (!res.vertices.empty()) {
                auto mesh = renderer->createChunkMesh(res.vertices);
                std::lock_guard<std::mutex> lock(world.meshMutex);
                world.meshes[{res.x, res.y}] = mesh;
            }
        }
    }

    void togglePause() {
        isPaused = !isPaused;
        glfwSetInputMode(window, GLFW_CURSOR, isPaused ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }

    void cleanup() {
        running = false;
        for (auto& t : workerThreads) {
            if (t.joinable()) t.join();
        }

        if (server) {
            server->stop();
        }

        if (renderer) {
            renderer->waitIdle();
            renderer->destroyChunkMesh(world.sunMesh);
            renderer->destroyChunkMesh(world.moonMesh);
        }
        for (auto& [pos, mesh] : world.meshes) {
            renderer->destroyChunkMesh(mesh);
        }
        renderer.reset();
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    static void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
        auto app = reinterpret_cast<MinecraftApp*>(glfwGetWindowUserPointer(window));
        if (app->state == GameState::PLAYING && !app->isPaused) {
            app->camera.handleMouseMovement(static_cast<float>(xpos), static_cast<float>(ypos));
        }
    }

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto app = reinterpret_cast<MinecraftApp*>(glfwGetWindowUserPointer(window));
        if (app->state == GameState::PLAYING) {
            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
                app->togglePause();
            }
            if (key == GLFW_KEY_F3 && action == GLFW_PRESS) {
                app->showDebugScreen = !app->showDebugScreen;
            }
        }
    }
};

int main() {
    MinecraftApp app;
    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
