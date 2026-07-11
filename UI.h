#pragma once

// Tell GLFW not to include the legacy OpenGL headers itself
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <vector>
#include "shader.h"


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
    float spinRadius = 5.0f;
    float lightPos[3] = {0.0f, 0.0f, 0.0f};
    float lightColor[3] = {1.0f, 1.0f, 1.0f};
    bool lightPulse = false;
    float pulseSpeed[3] = {1.0f, 1.0f, 1.0f};
};

// Function declarations
void InitUI(GLFWwindow* window);
void RenderUI(UIVariables& state, const std::vector<Shader*>& engineShaders);
void ShutdownUI();