#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <optional>
#include <chrono>
#include <cmath>

#include "World/World.hpp"
#include "World/ChunkMesher.hpp"
#include "Camera/Camera.hpp"
#include "Renderer/Renderer.hpp"
#include "Core/Logger.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

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

    void initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(1280, 720, "Minecraft Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        
        // Input Callbacks
        glfwSetCursorPosCallback(window, mouseCallback);
        glfwSetKeyCallback(window, keyCallback);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    void initApp() {
        RendererInitInfo initInfo{ window };
        renderer = std::make_unique<Renderer>(initInfo);
        
        // Initial World Gen
        updateWorld();
    }

    void mainLoop() {
        auto lastTime = std::chrono::high_resolution_clock::now();
        float totalTime = 0.0f;

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;
            totalTime += deltaTime;

            if (!isPaused) {
                camera.processInput(window, deltaTime, isPaused);
                updateWorld();
            }

            static int frameCount = 0;
            if (++frameCount % 60 == 0) {
                Logger::log("Camera Position: (" + std::to_string(camera.pos.x) + ", " + std::to_string(camera.pos.y) + ", " + std::to_string(camera.pos.z) + ")");
            }

            // 20 minute day cycle (1200 seconds)
            constexpr float dayLength = 1200.0f;
            float dayProgress = fmod(totalTime, dayLength) / dayLength;
            float angle = dayProgress * 2.0f * 3.14159f;

            // Sun direction (rotating around Y or X)
            glm::vec3 sunDirection = glm::vec3(sin(angle), -cos(angle), 0.2f);
            
            // Basic sun color based on time (night is dark, dawn/dusk is orange, day is white)
            glm::vec3 sunColor = glm::vec3(1.0f);
            float sunHeight = -sunDirection.y; // High is positive
            if (sunHeight < 0.0f) {
                // Night
                sunColor = glm::vec3(0.1f, 0.1f, 0.2f);
            } else if (sunHeight < 0.2f) {
                // Dawn/Dusk
                float t = sunHeight / 0.2f;
                sunColor = glm::mix(glm::vec3(1.0f, 0.4f, 0.2f), glm::vec3(1.0f, 1.0f, 1.0f), t);
            }

            if (!world.meshes.empty()) {
                renderer->drawFrame(world, camera, sunDirection, sunColor);
            }
        }
        renderer->waitIdle();
    }

    void updateWorld() {
        const int playerChunkX = static_cast<int>(std::floor(camera.pos.x / CHUNK_WIDTH));
        const int playerChunkY = static_cast<int>(std::floor(camera.pos.y / CHUNK_WIDTH));

        constexpr int radius = 2;
        for (int x = playerChunkX - radius; x <= playerChunkX + radius; ++x) {
            for (int y = playerChunkY - radius; y <= playerChunkY + radius; ++y) {
                if (world.chunks.find({x, y}) == world.chunks.end()) {
                    Logger::log("Generating chunk at (" + std::to_string(x) + ", " + std::to_string(y) + ")");
                    world.addChunk(x, y);
                    auto vertices = ChunkMesher::generateMesh(*world.getChunk(x, y), x, y, &world);
                    if (!vertices.empty()) {
                        Logger::log("Meshing chunk (" + std::to_string(x) + ", " + std::to_string(y) + ") - " + std::to_string(vertices.size()) + " vertices");
                        auto mesh = renderer->createChunkMesh(vertices);
                        
                        std::lock_guard<std::mutex> lock(world.meshMutex);
                        world.meshes[{x, y}] = mesh;
                    } else {
                        Logger::log("Warning: Chunk at (" + std::to_string(x) + ", " + std::to_string(y) + ") produced empty mesh");
                    }
                }
            }
        }
    }

    void togglePause() {
        isPaused = !isPaused;
        glfwSetInputMode(window, GLFW_CURSOR, isPaused ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }

    void cleanup() {
        if (renderer) renderer->waitIdle();
        for (auto& [pos, mesh] : world.meshes) {
            renderer->destroyChunkMesh(mesh);
        }
        renderer.reset();
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    static void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
        auto app = reinterpret_cast<MinecraftApp*>(glfwGetWindowUserPointer(window));
        if (!app->isPaused) {
            app->camera.handleMouseMovement(static_cast<float>(xpos), static_cast<float>(ypos));
        }
    }

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto app = reinterpret_cast<MinecraftApp*>(glfwGetWindowUserPointer(window));
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            app->togglePause();
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
