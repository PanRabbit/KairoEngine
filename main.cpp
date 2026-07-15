#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>

#include <stb/stb_image.h>
#include <kairo/UI.h>
#include <kairo/shader.h>
#include <kairo/camera.h>
#include <kairo/texture.h>
#include <kairo/material.h>

float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

float SCR_WIDTH = 1024;
float SCR_HEIGHT = 1024;



// ====================================
// CAMERA VARIABLES SETUP
// ====================================
                        // pos                          //rot
Camera camera(glm::vec3(3.5f, 1.0f, -5.0f), glm::vec3(-0.55f, -0.05f, 0.83f));

// ====================================
// WINDOW SETUP
// ====================================

// handle window resize
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// process input
void processInput(GLFWwindow *window)
{   
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Camera Speed modifier (Sprint)
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.MovementSpeed = 2.5f;
    else
        camera.MovementSpeed = 1.0f;

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
    static bool tabPressedLastFrame = false;
    bool tabPressedNow = (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS);
    if (tabPressedNow && !tabPressedLastFrame) 
    {
        int currentMode = glfwGetInputMode(window, GLFW_CURSOR);
        glfwSetInputMode(window, GLFW_CURSOR, (currentMode == GLFW_CURSOR_NORMAL) ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
    tabPressedLastFrame = tabPressedNow; 
}

// Mouse variables can stay global since they only track the physical mouse state
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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "KairoEngine", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    //set window states
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSwapInterval(0); //vsync

    // init glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }


    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glEnable(GL_DEPTH_TEST); //enable depth testing

    // ==========================================
    // SHADER COMPILATION & LINKING
    // ==========================================
  
    Shader phongShader("shaders/vertex_shader.glsl", "shaders/phong_shader.glsl");
    Shader lightShader("shaders/vertex_shader.glsl", "shaders/light_shader.glsl");

    std::vector<Shader*> allShaders = {&lightShader,  &phongShader};

    // ==========================================
    // VERTEX DATA & BUFFERS (VAO & VBO)
    // ==========================================
    float vertices[] = {
        // --- CUBE (36 vertices) ---
        // Positions          // Normals          // Tex Coords
        // Back face
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        // Front face
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
        // Left face
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        // Right face
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        // Bottom face
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
        // Top face
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,

        // --- FLOOR (6 vertices) ---
        -10.0f, -1.0f, -10.0f,  0.0f, 1.0f, 0.0f,  0.0f,  0.0f,
        10.0f, -1.0f, -10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 0.0f,
        10.0f, -1.0f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f,
        10.0f, -1.0f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f,
        -10.0f, -1.0f,  10.0f,  0.0f, 1.0f, 0.0f,  0.0f,  10.0f,
        -10.0f, -1.0f, -10.0f,  0.0f, 1.0f, 0.0f,  0.0f,  0.0f
    };
    

    unsigned int VAO, VBO; //, EBO;
    glGenVertexArrays(1, &VAO); 
    glGenBuffers(1, &VBO); 
    //glGenBuffers(1, &EBO); 

    glBindVertexArray(VAO); 

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),(void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);   

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); 

    // Light VAO
    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);

    glBindVertexArray(lightVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    // ==========================================
    // MATERIALS
    // ==========================================

    Material woodMaterial(&phongShader);
    woodMaterial.loadFromJson("materials/container.json");

    Material floorMaterial(&phongShader);
    floorMaterial.loadFromJson("materials/floor.json");

    Material mikuMaterial(&phongShader);
    mikuMaterial.loadFromJson("materials/mikuCube.json");

    Material emeraldMaterial(&phongShader);
    emeraldMaterial.loadFromJson("materials/emerald.json");

    Material goldMaterial(&phongShader);
    goldMaterial.loadFromJson("materials/gold.json");

    Material greyCheckerMaterial(&phongShader);
    greyCheckerMaterial.loadFromJson("materials/greyChecker.json");

    Material grungeMaterial(&phongShader);
    grungeMaterial.loadFromJson("materials/grunge.json");

    Material testMaterial(&phongShader);
    testMaterial.loadFromJson("materials/test.json");



    std::vector<Material*> allMaterials  = {&woodMaterial,  &floorMaterial, &mikuMaterial, &testMaterial, &emeraldMaterial, &goldMaterial, &greyCheckerMaterial, &grungeMaterial };

    // ==========================================
    // MATRICES & VECTORS
    // ==========================================

    // model matrix, transforms all objects vertices to world spcace. slightly rotated here to lay on the floor
    glm::mat4 model = glm::mat4(1.0f);
    
    // view matrix (basically the camera I think?) left blank to be set per frame
    glm::mat4 view;

    // projection matrix (make it not ortho), ,moved to render loop
    glm::mat4 projection;
    
    // ==========================================
    // OBJECTS
    // ==========================================
    // random coords
    // glm::vec3 cubePositions[] = {
    //     glm::vec3( 0.0f, -1.0f, 0.0f),
    //     glm::vec3( 2.0f, 5.0f, -5.0f),
    //     glm::vec3(-1.5f, -2.2f, -2.5f),
    //     glm::vec3(-3.8f, -2.0f, 2.3f),
    //     glm::vec3( 2.4f, -0.4f, -3.5f),
    //     glm::vec3(-1.7f, 3.0f, 3.5f),
    //     glm::vec3( 1.3f, -2.0f, -2.5f),
    //     glm::vec3( 1.5f, 2.0f, -2.5f),
    //     glm::vec3( 1.5f, 0.2f, 1.5f),
    //     glm::vec3(-1.3f, 1.0f, 1.5f)
    //     };

    // grid
    glm::vec3 cubePositions[] = {
        glm::vec3(-2.0f, 0.0f, 0.0f),
        glm::vec3(-1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(-2.0f, 1.0f, 0.0f),
        glm::vec3(-1.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 0.0f),

    };

    glm::vec3 lightPos;

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
        float currentFrame = (glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // Process UI toggles 
        if (state.isWireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        
        // Clear Screen
        glClearColor(state.clearColor[0], state.clearColor[1], state.clearColor[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        

        // Calculate Variables for this frame
        view = camera.GetViewMatrix();
        projection = glm::perspective(glm::radians(camera.Zoom), SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);


        // Update Light Position and Color
        if (state.lightSpin)
        {
            lightPos = glm::vec3(sin(glfwGetTime() * state.spinSpeed) * state.spinRadius, 0.0f, cos(glfwGetTime() * state.spinSpeed) * state.spinRadius);
        }
        else {
            lightPos = glm::vec3(state.lightPos[0], state.lightPos[1], state.lightPos[2]);
        }
        

        glm::vec3 lightColor;
        if (state.lightPulse) {
            lightColor = glm::vec3(
                glm::max(0.1f, (float)sin(glfwGetTime() * state.pulseSpeed[0])), 
                glm::max(0.1f, (float)sin(glfwGetTime() * state.pulseSpeed[1])), 
                glm::max(0.1f, (float)sin(glfwGetTime() * state.pulseSpeed[2]))
            );        
        } 
        else {
            lightColor = glm::vec3(state.lightColor[0], state.lightColor[1], state.lightColor[2]);
        }


        // ==========================================
        // SETUP LIGHT
        //==========================================
        phongShader.use();

        phongShader.setVec3("light.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
        phongShader.setVec3("light.diffuse", lightColor);
        phongShader.setVec3("light.specular", glm::vec3(1.0f, 1.0f, 1.0f));
        phongShader.setVec3("light.position", lightPos);

        // ==========================================
        // DRAW CUBES
        // ==========================================
        // 1. Let the Material do ALL the heavy lifting (Activates shader, binds textures, sets uniforms)
        //woodMaterial.apply();
        testMaterial.apply();

        // 2. Set the global/frame variables (Lights and Camera)
        phongShader.setVec3("light.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
        phongShader.setVec3("light.diffuse", lightColor);
        phongShader.setVec3("light.specular", glm::vec3(1.0f, 1.0f, 1.0f));
        phongShader.setVec3("light.position", lightPos);

        phongShader.setMat4("view", view);
        phongShader.setMat4("projection", projection);
        phongShader.setVec3("viewPos", camera.Position);

        glBindVertexArray(VAO); 

        // 3. Draw the objects!
        for(unsigned int i = 0; i < allMaterials.size(); i++)
        {   
            allMaterials[i]->apply();
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            model = glm::scale(model, glm::vec3(0.9f)); 
            float angle = 20.0f * i;
            //model = glm::rotate(model, glm::radians(currentFrame * (angle + 1) * 0.1f), glm::vec3(0.5f + angle, 1.0f + angle, 1.0f + angle));

            phongShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // ==========================================
        // DRAW FLOOR
        // ==========================================
        // 1. Swap to the floor material!
        floorMaterial.apply();

        // 2. We don't need to re-send the camera/light data because both materials 
        // use the same phongShader, and OpenGL remembers the state! 
        glm::mat4 floorModel = glm::mat4(1.0f); 
        floorModel = glm::translate(floorModel, glm::vec3(0.0f, -2.0f, 0.0f));
        phongShader.setMat4("model", floorModel); 

        glDrawArrays(GL_TRIANGLES, 36, 6);   

        // ==========================================
        // DRAW LIGHT CUBE
        // ==========================================

        lightShader.use();
        lightShader.setVec3("Color", lightColor);
        lightShader.setMat4("view", view);
        lightShader.setMat4("projection", projection);
        glm::mat4 lightModel = glm::mat4(1.0f); 
        lightModel = glm::translate(lightModel, lightPos); 
        lightModel = glm::scale(lightModel, glm::vec3(0.2f)); 
        lightShader.setMat4("model", lightModel);

        glBindVertexArray(lightVAO); 
        glDrawArrays(GL_TRIANGLES, 0, 36);


       // ==========================================
        // END FRAME UI & SWAP
        // ==========================================
        state.cameraSpeed = camera.MovementSpeed;
        state.cameraPos = camera.Position;
        state.cameraRot = camera.Front;
        state.fov = camera.Zoom;
        
        RenderUI(state, allShaders, allMaterials);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    // ========= END RENDER LOOP =========

    // Clean up allocated resources safely 
    ShutdownUI();

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    //glDeleteBuffers(1, &EBO);


    glfwTerminate();
    return 0;
}