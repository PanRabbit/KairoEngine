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
#include "src/kairo/level_definition.cpp"
#include "src/kairo/asset_load.cpp"

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

    glViewport(0, 0, static_cast<int>(screenWidth), static_cast<int>(screenHeight));
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);


    // ==========================================
    // POSTPROCESSING INIT
    // ==========================================

    //create framebuffer
    engineContext.postProcessingFB = 0;
    glGenFramebuffers(1, &engineContext.postProcessingFB);
    glBindFramebuffer(GL_FRAMEBUFFER, engineContext.postProcessingFB);

    //create color texture
    engineContext.texColorBuffer = 0;
    glGenTextures(1, &engineContext.texColorBuffer);
    glBindTexture(GL_TEXTURE_2D, engineContext.texColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<int>(screenWidth), static_cast<int>(screenHeight), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    //attach texture to currently bound framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, engineContext.texColorBuffer, 0);

    //create renderbuffer object for depth and stencil attachment
    engineContext.rboDepthStencil = 0;
    glGenRenderbuffers(1, &engineContext.rboDepthStencil);
    glBindRenderbuffer(GL_RENDERBUFFER, engineContext.rboDepthStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, static_cast<int>(screenWidth), static_cast<int>(screenHeight));
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    //attach renderbuffer to framebuffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, engineContext.rboDepthStencil);

    //check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    float PPQuadVertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    engineContext.PPVBO = 0;
    glGenBuffers(1, &engineContext.PPVBO);
    glBindBuffer(GL_ARRAY_BUFFER, engineContext.PPVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(PPQuadVertices), PPQuadVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    engineContext.PPVAO = 0;
    glGenVertexArrays(1, &engineContext.PPVAO);
    glBindVertexArray(engineContext.PPVAO);
    glBindBuffer(GL_ARRAY_BUFFER, engineContext.PPVBO);

    // set up vertex attributes
        // positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        // texcoords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // ==========================================
    // ASSET LOADING
    // ==========================================
    AssetLoad(engineContext);

    // ==========================================
    // LEVEL DEFINITION (after assets loaded)
    // ==========================================
    DefineLevel(engineContext);

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