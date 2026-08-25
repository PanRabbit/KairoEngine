#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>


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
#include "src/kairo/post_process.cpp"
#include "src/kairo/asset_load.cpp"
#include "src/kairo/render_loop.cpp"
#include "src/kairo/level_definition.cpp"
#include <kairo/engine_context.h>

float screenWidth = 1600;
float screenHeight = 1200;

bool flashlightOn = false;
int selectedObjectID = 0;



// ====================================
// CAMERA SETUP
// ====================================
                        // pos                          //rot
Camera camera(glm::vec3(3.5f, 1.0f, -5.0f), glm::vec3(-0.55f, -0.05f, 0.83f));

// ===================================
// MAIN FUNCTION
// ===================================

int main()
{
    // ==========================================
    // GLFW INITIALIZATION
    // ==========================================
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // multisampling for anti-aliasing


    // ==========================================
    // WINDOW CREATION
    // ==========================================
    GLFWwindow* window = glfwCreateWindow(static_cast<int>(screenWidth), static_cast<int>(screenHeight), "KairoEngine", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // ==========================================
    // GLAD INITIALIZATION
    // ==========================================
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // ==========================================
    // ENGINE CONTEXT INITIALIZATION 
    // ==========================================
    // init BEFORE setting intput callbacks
    EngineContext engineContext;
    engineContext.camera = camera;


    // ==========================================
    // SELECTION FRAMEBUFFER CREATION
    // ==========================================
    engineContext.selectionFB.init(static_cast<int>(screenWidth), static_cast<int>(screenHeight));


    // ==========================================
    // WINDOW STATES
    // ==========================================

    // Set global context pointer for GLFW callbacks
    setInputContext(&engineContext);

    // set window states (uses global context pointer internally)
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSwapInterval(0); // vsync
    glEnable(GL_MULTISAMPLE); // enable multisampling for anti-aliasing


    glViewport(0, 0, static_cast<int>(screenWidth), static_cast<int>(screenHeight));
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // ==========================================
    // POSTPROCESSING INIT
    // ==========================================
    PostProcess(engineContext);

    // ==========================================
    // ASSET LOADING
    // ==========================================
    AssetLoad(engineContext);

    // ==========================================
    // LEVEL DEFINITION (after assets loaded, dk how I mande that mistake lol)
    // ==========================================
    LoadLevelFromJson(engineContext, "levels/day.json");

    // ==========================================
    // UI INITIALIZATION
    // ==========================================
    InitUI(window);

    // ==========================================
    // RENDER LOOP
    // ==========================================
    while(!glfwWindowShouldClose(window))
    {
        RenderLoop(window, engineContext);
    }

    // Clean up allocated resources
    ShutdownUI();
    engineContext.selectionFB.cleanup();

    glfwTerminate();
    return 0;
}