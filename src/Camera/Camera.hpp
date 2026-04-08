#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <algorithm>

class Camera {
public:
    glm::vec3 pos = glm::vec3(8.0f, 8.0f, 30.0f);
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 velocity = glm::vec3(0.0f);
    float yaw = -90.0f;
    float pitch = -45.0f;
    float lastX = 640.0f, lastY = 360.0f;
    bool firstMouse = true;

    void setPosition(const glm::vec3& newPos) {
        pos = newPos;
    }

    void processInput(GLFWwindow* window, float deltaTime, bool isPaused) {
        if (isPaused) {
            velocity = glm::vec3(0.0f);
            return;
        }

        const float acceleration = 50.0f;
        const float friction = 8.0f;
        const float maxSpeed = 10.0f;

        glm::vec3 inputDir = glm::vec3(0.0f);
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) inputDir += front;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) inputDir -= front;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) inputDir -= glm::normalize(glm::cross(front, up));
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) inputDir += glm::normalize(glm::cross(front, up));
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) inputDir += up;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) inputDir -= up;

        if (glm::length(inputDir) > 0.0f) {
            inputDir = glm::normalize(inputDir);
            velocity += inputDir * acceleration * deltaTime;
        }

        // Apply friction
        velocity -= velocity * friction * deltaTime;

        // Clamp speed
        if (glm::length(velocity) > maxSpeed) {
            velocity = glm::normalize(velocity) * maxSpeed;
        }

        pos += velocity * deltaTime;
    }

    void handleMouseMovement(float xpos, float ypos) {
        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = lastX - xpos;
        float yoffset = lastY - ypos;
        lastX = xpos;
        lastY = ypos;

        constexpr float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        pitch = std::clamp(pitch, -89.0f, 89.0f);

        updateCameraVectors();
    }

    [[nodiscard]] glm::mat4 getViewMatrix() const {
        return glm::lookAt(pos, pos + front, up);
    }

private:
    void updateCameraVectors() {
        glm::vec3 direction;
        direction.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
        direction.y = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));
        direction.z = std::sin(glm::radians(pitch));
        front = glm::normalize(direction);
    }
};
