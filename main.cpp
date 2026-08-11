#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <vector>

#include <stb/stb_image.h>

#include <kairo/UI.h>
#include <kairo/shader.h>
#include <kairo/camera.h>
#include <kairo/texture.h>
#include <kairo/material.h>
#include <kairo/mesh.h>
#include <kairo/model.h>
#include <kairo/game_object.h>
#include <kairo/selection.h>
#include <kairo/input.h>
#include <kairo/engine_context.h>
#include "src/kairo/render_loop.cpp"

float screenWidth = 1600;
float screenHeight = 1200;

bool flashlightOn = false;
int selectedObjectID = 0;


// ====================================
// CAMERA VARIABLES SETUP
// ====================================
                        // pos                          //rot
Camera camera(glm::vec3(3.5f, 1.0f, -5.0f), glm::vec3(-0.55f, -0.05f, 0.83f));

// ===================================
// MAIN FUNCTION
// ===================================

int main()
{
    // init glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // create window object
    GLFWwindow* window = glfwCreateWindow(static_cast<int>(screenWidth), static_cast<int>(screenHeight), "KairoEngine", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // create selection framebuffer (before engine context init)
    SelectionBuffer selectionFB;
    selectionFB.init(static_cast<int>(screenWidth), static_cast<int>(screenHeight));

    // ==========================================
    // SHADER & MATERIAL INITIALIZATION
    // ==========================================
    ShadersAndMaterials(engineContext);

    // ==========================================
    // MATRICES & VECTORS
    // ==========================================
    glm::mat4 view;
    glm::mat4 projection;

    // ==========================================
    // LEVEL DEFINITION
    // ==========================================

    DefineLevel(engineContext);

    // ==========================================
    // ENGINE CONTEXT INITIALIZATION 
    // ==========================================
    // init BEFORE setting intput callbacks
    EngineContext engineContext{camera, selectionFB, selectionShader, sceneObjects, view, projection, deltaTime, screenWidth, screenHeight, flashlightOn, selectedObjectID};

    // Set global context pointer for GLFW callbacks
    setInputContext(&engineContext);

    // set window states (uses global context pointer internally)
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSwapInterval(0); // vsync

    glViewport(0, 0, static_cast<int>(screenWidth), static_cast<int>(screenHeight));
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // ==========================================
    // UI INITIALIZATION
    // ==========================================
    InitUI(window);

    // ==========================================
    // RENDER LOOP
    // ==========================================
    while(!glfwWindowShouldClose(window))
    {
        RenderLoop(engineContext, allShaders, allMaterials);
    }

    // Clean up allocated resources
    ShutdownUI();
    selectionFB.cleanup();

    meshSuzanne.cleanup();
    meshCube.cleanup();
    meshPlane.cleanup();
    meshSphere.cleanup();

    glfwTerminate();
    return 0;
}