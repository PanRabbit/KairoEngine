#pragma once

// Tell GLFW not to include the legacy OpenGL headers itself
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <vector>
#include <kairo/shader.h>
#include <kairo/material.h>

// A struct to hold variables that the UI will modify
struct UIVariables {
    bool isWireframe = false;
    float clearColor[3] = { 0.04f, 0.03f, 0.06f };
    bool reloadShader = false;
    // You can easily add more variables here later (e.g., float cameraSpeed, int objectCount)

    float cameraSpeed = 3.0f, fov = 45.0f;
    glm::vec3 cameraPos, cameraRot;

    bool lightSpin = true;
    float spinSpeed = 1.0f;
    float spinRadius = 3.0f;
    float lightPos[3] = {-1.5f, -0.3f, -4.5f};
    float lightColor[3] = {1.0f, 1.0f, 1.0f};
    bool lightPulse = false;
    float pulseSpeed[3] = {1.0f, 1.0f, 1.0f};
};

// Function declarations
void InitUI(GLFWwindow* window);
void RenderUI(UIVariables& state, const std::vector<Shader*>& engineShaders, const std::vector<Material*>& engineMaterials);
void ShutdownUI();