#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <optional>
#include <memory>
#include "World/Vertex.hpp"
#include "World/World.hpp"
#include "Camera/Camera.hpp"

struct RendererInitInfo {
    GLFWwindow* window;
};

class Renderer {
public:
    static constexpr int kMaxFramesInFlight = 2;
    Renderer(const RendererInitInfo& info);
    ~Renderer();

    void drawFrame(const World& world, const Camera& camera, glm::vec3 sunDirection, glm::vec3 sunColor);
    void waitIdle();
    void recreateSwapChain();
    void setFramebufferResized() { framebufferResized = true; }

    // Helper for Chunk Meshing (GPU buffer creation)
    ChunkMesh createChunkMesh(const std::vector<Vertex>& vertices);
    void destroyChunkMesh(ChunkMesh& mesh);

private:
    // Vulkan Core
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;

    // Swap Chain
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    // Depth Buffer
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    // Pipeline
    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    // Buffers & Descriptors
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    // Sync Objects
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;
    bool framebufferResized = false;

    GLFWwindow* window;

    // Internal Init Methods
    void initVulkan();
    void createInstance();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createDescriptorSetLayout();
    void createGraphicsPipeline();
    void createDepthResources();
    void createFramebuffers();
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();

    // Cleanup
    void cleanupSwapChain();
    void cleanup();

    // Helpers
    void updateUniformBuffer(uint32_t currentImage, const Camera& camera, glm::vec3 sunDirection, glm::vec3 sunColor);
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, const World& world);
    VkShaderModule createShaderModule(const std::vector<char>& code);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    VkFormat findDepthFormat();
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    
    // Static callbacks
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};
