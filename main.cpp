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

float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

float SCR_WIDTH = 1600;
float SCR_HEIGHT = 1200;

bool flashlightOn = false;
int selectedObjectID = 0;


// ====================================
// CAMERA VARIABLES SETUP
// ====================================
                        // pos                          //rot
Camera camera(glm::vec3(3.5f, 1.0f, -5.0f), glm::vec3(-0.55f, -0.05f, 0.83f));

// ====================================
// PICKING BUFFER STRUCT & FUNCTION
// ====================================
struct PickingBuffer {
    unsigned int fbo = 0;
    unsigned int colorTexture = 0;
    unsigned int depthBuffer = 0;

    void init(int width, int height) {
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        // Color Texture (Stores IDs)
        glGenTextures(1, &colorTexture);
        glBindTexture(GL_TEXTURE_2D, colorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

        // Depth Buffer (Needed so overlapping geometry occludes properly!)
        glGenRenderbuffers(1, &depthBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void cleanup() {
        if (fbo) glDeleteFramebuffers(1, &fbo);
        if (colorTexture) glDeleteTextures(1, &colorTexture);
        if (depthBuffer) glDeleteRenderbuffers(1, &depthBuffer);
    }
};

int PerformSelection(double mouseX, double mouseY, int screenWidth, int screenHeight, 
    Shader& pickingShader, PickingBuffer& pickingFB, 
    const std::vector<GameObject*>& sceneObjects, 
    const glm::mat4& view, const glm::mat4& projection) 
{
    // Bind our offscreen framebuffer so we render to a texture instead of the screen
    glBindFramebuffer(GL_FRAMEBUFFER, pickingFB.fbo);
    glViewport(0, 0, screenWidth, screenHeight);
    glEnable(GL_DEPTH_TEST);

    // Clear to black (ID = 0 means "nothing was clicked")
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up the picking shader with camera matrices
    pickingShader.use();
    pickingShader.setMat4("view", view);
    pickingShader.setMat4("projection", projection);

    // Render every object — each one writes its own ID into the fragment color
    for (const auto* obj : sceneObjects) {
        obj->drawPicking(pickingShader);
    }

    // Read the single pixel under the mouse. OpenGL Y is flipped relative to GLFW, so we flip it back.
    int flippedY = screenHeight - static_cast<int>(mouseY);
    unsigned char pixel[3];
    glReadPixels(static_cast<int>(mouseX), flippedY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);

    // Unbind the FBO so rendering goes back to the screen normally
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Decode the RGB color back into a single integer ID (R + G*256 + B*65536)
    return pixel[0] + (pixel[1] * 256) + (pixel[2] * 256 * 256);
}

// ====================================
// WINDOW SETUP
// ====================================

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// process input
void processInput(GLFWwindow *window, Shader& pickingShader, PickingBuffer& pickingFB, 
                  const std::vector<GameObject*>& sceneObjects,
                  const glm::mat4& view, const glm::mat4& projection)
{   
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Camera Speed modifier (Sprint)
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.MovementSpeed = 2.5f;
    else
        camera.MovementSpeed = 1.0f;

    // torch
    static bool fJustPressed = false;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    {
        if (!fJustPressed)
        {
            flashlightOn = !flashlightOn;
            fJustPressed = true;
        }
    }
    else
    {
        fJustPressed = false;
    }

    // Movement
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);

    // toggle mouse lock
    static bool tabJustPressed = false;
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) 
    {
        if (!tabJustPressed) 
        {
            int currentMode = glfwGetInputMode(window, GLFW_CURSOR);
            glfwSetInputMode(window, GLFW_CURSOR, (currentMode == GLFW_CURSOR_NORMAL) ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
            tabJustPressed = true;
        }
    }
    else
    {
        tabJustPressed = false;
    }

    // Mouse click selection (Only trigger when cursor is not captured by camera look)
    static bool mouseJustPressed = false;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if (!mouseJustPressed && glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL)
        {
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);

            int selectedID = PerformSelection(mouseX, mouseY, static_cast<int>(SCR_WIDTH), static_cast<int>(SCR_HEIGHT),
                                              pickingShader, pickingFB, sceneObjects, view, projection);
            
            std::cout << "Clicked Object ID: " << selectedID << std::endl;
            selectedObjectID = selectedID;
            mouseJustPressed = true;
        }
    }
    else
    {
        mouseJustPressed = false;
    }
}

// Mouse variables
float mouseLastX = SCR_WIDTH / 2.0f;
float mouseLastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL)
    {
        firstMouse = true;
        return;
    }

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        mouseLastX = xpos;
        mouseLastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - mouseLastX;
    float yoffset = mouseLastY - ypos; // reversed since y-coordinates go from bottom to top

    mouseLastX = xpos;
    mouseLastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// scroll to change fov
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

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
    GLFWwindow* window = glfwCreateWindow(static_cast<int>(SCR_WIDTH), static_cast<int>(SCR_HEIGHT), "KairoEngine", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // set window states
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSwapInterval(0); // vsync

    // init glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, static_cast<int>(SCR_WIDTH), static_cast<int>(SCR_HEIGHT));
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Initialize Picking Framebuffer
    PickingBuffer pickingFB;
    pickingFB.init(static_cast<int>(SCR_WIDTH), static_cast<int>(SCR_HEIGHT));

    // ==========================================
    // SHADER COMPILATION & LINKING
    // ==========================================
    Shader phongShader("shaders/vertex_shader.glsl", "shaders/phong_shader.glsl");
    Shader lightShader("shaders/vertex_shader.glsl", "shaders/light_shader.glsl");
    Shader depthShader("shaders/vertex_shader.glsl", "shaders/depth_visualiser.fs");
    Shader pickingShader("shaders/picking.vs", "shaders/picking.fs");

    std::vector<Shader*> allShaders = {&lightShader, &phongShader};

    // ==========================================
    // MATERIALS
    // ==========================================
    Material woodMaterial(&phongShader);
    woodMaterial.loadFromJson("materials/container.json");

    Material floorMaterial(&phongShader);
    floorMaterial.loadFromJson("materials/floor.json");

    Material mikuMaterial(&phongShader);
    mikuMaterial.loadFromJson("materials/mikuCube.json");

    Material grungeMaterial(&phongShader);
    grungeMaterial.loadFromJson("materials/grunge.json");

    Material lightMaterial(&lightShader);
    lightMaterial.loadFromJson("materials/light.json");

    std::vector<Material*> allMaterials = {&woodMaterial, &floorMaterial, &mikuMaterial, &grungeMaterial, &lightMaterial};

    // ==========================================
    // MATRICES & VECTORS
    // ==========================================
    glm::mat4 view;
    glm::mat4 projection;
    
    // ==========================================
    // OBJECT POSITIONS
    // ==========================================

    std::vector<GameObject*> sceneObjects;

    glm::vec3 cubePositions[] = {
        glm::vec3( 2.6f, 0.5f, 0.24f),
        glm::vec3( 1.7f, 1.0f, 1.96f),
        glm::vec3( 1.7f, 2.5f, 1.96f),
        glm::vec3( -0.67f, 0.5f, 2.0f),
        glm::vec3( -2.2f, 0.5f, 0.6f),
        glm::vec3( -2.8f, 0.5f, -0.23f),
        glm::vec3( -2.5f, 1.375f, 0.2f)
    };

    float cubeRotations[] = { -28.0f, 37.0f, 60.0f, -16.0f, -54.0f, -54.0f, -30.0f };

    float cubeScales[] = { 1.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.75f };

    // Point Lights
    glm::vec3 pointLightPositions[] = {
        glm::vec3( 2.0f, 1.5f, -0.85f),
        glm::vec3( -1.5f, 2.2f, 1.7f),
        glm::vec3( -1.8f, 0.5f, -1.5f),
    };
    glm::vec3 pointLightColors[] = {
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(0.19f, 0.75f, 1.0f),
        glm::vec3(1.0f, 0.33f, 0.43f),
    };
    float pointLightIntensityMults[] = { 8.0f, 16.0f, 4.0f };

    glm::vec3 sunDirection = glm::vec3(-0.2f, -1.0f, 0.5f);
    glm::vec3 lightColor;
    glm::vec3 torchColor = glm::vec3(1.0f, 0.95f, 0.8f);

    Model meshSuzanne("meshes/suzanne.obj");
    Model meshCube("meshes/prims/cube.obj");
    Model meshPlane("meshes/prims/plane.obj");
    Model meshSphere("meshes/prims/sphere.obj");

    for (int i = 0; i < 7; i++) {
        auto* cube = new GameObject("Cube_" + std::to_string(i), &meshCube, &woodMaterial);
        cube->position = cubePositions[i];
        cube->rotation.y = glm::degrees(cubeRotations[i]);
        cube->scale = glm::vec3(cubeScales[i]);
        sceneObjects.push_back(cube);
    }   

    auto* floorObj = new GameObject("Floor", &meshPlane, &floorMaterial);
    floorObj->scale = glm::vec3(10.0f);
    sceneObjects.push_back(floorObj);

    auto* suzanneObj = new GameObject("Suzanne", &meshSuzanne, &grungeMaterial);
    suzanneObj->position = glm::vec3(0.0f, 2.0f, 0.0f);
    suzanneObj->rotation.y = glm::degrees(180.0f);
    suzanneObj->scale = glm::vec3(0.5f);
    sceneObjects.push_back(suzanneObj);

    // ==========================================
    // UI INITIALIZATION
    // ==========================================
    UIVariables state;
    InitUI(window);

    // ==========================================
    // RENDER LOOP
    // ==========================================
    while(!glfwWindowShouldClose(window))
    {
        // Process time & Input
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Calculate Matrices for this frame
        view = camera.GetViewMatrix();
        projection = glm::perspective(glm::radians(camera.Zoom), SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);

        // Process UI toggles 
        if (state.isWireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        // ==========================================
        // LIGHTS AND CAMERA VARIABLES
        // ==========================================
        
        phongShader.use();

        // Directional light (sun)
        glm::vec3 sunColor = glm::vec3(state.clearColor[0], state.clearColor[1], state.clearColor[2]);
        phongShader.setVec3("dirLight.direction", sunDirection);
        phongShader.setFloat("dirLight.intensity", 5.0f);
        phongShader.setVec3("dirLight.ambient", sunColor * 0.15f);
        phongShader.setVec3("dirLight.diffuse", sunColor * 1.0f);
        phongShader.setVec3("dirLight.specular", sunColor * 1.0f);

        // Point lights
        for(unsigned int i = 0; i < 3; i++)
        {
            lightColor = pointLightColors[i];
            std::string uniformID = "pointLights[" + std::to_string(i) + "].";
            phongShader.setVec3(uniformID + "ambient", lightColor * 0.15f);
            phongShader.setVec3(uniformID + "diffuse", lightColor * 1.0f);
            phongShader.setVec3(uniformID + "specular", lightColor * 1.0f);
            phongShader.setVec3(uniformID + "position", pointLightPositions[i]);
            phongShader.setFloat(uniformID + "radius", 8.0f);
            phongShader.setFloat(uniformID + "intensity", 0.5f * pointLightIntensityMults[i]);
        }

        // Flashlight
        if (flashlightOn)
        {
            phongShader.setVec3("spotLight.position", camera.Position);
            phongShader.setVec3("spotLight.direction", camera.Front);
            phongShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
            phongShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(20.0f)));
            phongShader.setVec3("spotLight.ambient", torchColor * 0.15f);
            phongShader.setVec3("spotLight.diffuse", torchColor * 1.0f);
            phongShader.setVec3("spotLight.specular", torchColor * 1.0f);
            phongShader.setFloat("spotLight.radius", 64.0f);
            phongShader.setFloat("spotLight.intensity", 32.0f);
        }
        else
        {
            phongShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(0.0f)));
            phongShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(0.0f)));
            phongShader.setFloat("spotLight.intensity", 0.0f);
        }

        // Camera matrices
        phongShader.setMat4("view", view);
        phongShader.setMat4("projection", projection);
        phongShader.setVec3("viewPos", camera.Position);

        // ==========================================
        // BEGIN DRAW
        // ==========================================
        glClearColor(sunColor.r, sunColor.g, sunColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render pass — just iterate objects directly:
        for (auto* obj : sceneObjects) {
            obj->draw(phongShader, selectedObjectID);
        }

        // DRAW LIGHT SPHERES
        lightShader.use();
        lightShader.setMat4("view", view);
        lightShader.setMat4("projection", projection);

        for(unsigned int i = 0; i < 3; i++)
        {   
            lightColor = pointLightColors[i];
            lightShader.setVec3("Color", lightColor);
            glm::mat4 lightModel = glm::mat4(1.0f); 
            lightModel = glm::translate(lightModel, pointLightPositions[i]); 
            lightModel = glm::scale(lightModel, glm::vec3(0.2f)); 
            lightShader.setMat4("model", lightModel);
            meshSphere.draw(lightMaterial);
        }

        // ==========================================
        // END FRAME UI & SWAP
        // ==========================================
        state.cameraSpeed = camera.MovementSpeed;
        state.cameraPos = camera.Position;
        state.cameraRot = camera.Front;
        state.fov = camera.Zoom;
        processInput(window, pickingShader, pickingFB, sceneObjects, view, projection);
        RenderUI(state, allShaders, allMaterials);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    // Clean up allocated resources
    ShutdownUI();
    pickingFB.cleanup();

    meshSuzanne.cleanup();
    meshCube.cleanup();
    meshPlane.cleanup();
    meshSphere.cleanup();

    glfwTerminate();
    return 0;
}