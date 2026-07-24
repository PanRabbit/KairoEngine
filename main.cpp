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
#include <kairo/mesh.h>
#include <kairo/model.h>

float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

float SCR_WIDTH = 1600;
float SCR_HEIGHT = 1200;

bool flashlightOn = false;

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
        -10.0f, 0.0f, -10.0f,  0.0f, 1.0f, 0.0f,  0.0f,  0.0f,
        10.0f, 0.0f, -10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 0.0f,
        10.0f, 0.0f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f,
        10.0f, 0.0f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f,
        -10.0f, 0.0f,  10.0f,  0.0f, 1.0f, 0.0f,  0.0f,  10.0f,
        -10.0f, 0.0f, -10.0f,  0.0f, 1.0f, 0.0f,  0.0f,  0.0f
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

    Material grungeMaterial(&phongShader);
    grungeMaterial.loadFromJson("materials/grunge.json");

    Material ainzMaterial(&phongShader);
    ainzMaterial.loadFromJson("materials/ainz.json");


    std::vector<Material*> allMaterials  = {&woodMaterial,  &floorMaterial, &mikuMaterial, &grungeMaterial };

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
    // random cubes
    glm::vec3 cubePositions[] = {
        glm::vec3( 2.6f, 0.5f, 0.24f),
        glm::vec3( 1.7f, 1.0f, 1.96f),
        glm::vec3( 1.7f, 2.5f, 1.96f),
        glm::vec3( -0.67f, 0.5f, 2.0f),
        glm::vec3( -2.2f, 0.5f, 0.6f),
        glm::vec3( -2.8f, 0.5f, -0.23f),
        glm::vec3( -2.5f, 1.375f, 0.2f)
        };

    float cubeRotations[] = {
        glm::radians(-28.0f),
        glm::radians(37.0f),
        glm::radians(60.0f),
        glm::radians(-16.0f),
        glm::radians(-54.0f),
        glm::radians(-54.0f),
        glm::radians(-30.0f)
        };

    float cubeScales[] = {
        1.0f,
        2.0f,
        1.0f,
        1.0f,
        1.0f,
        1.0f,
        0.75f
    };

    // point lights
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
    float pointLightIntensityMults[] = {
        4.0f,
        16.0f,
        2.0f,
    };


    glm::vec3 sunColor;
    glm::vec3 sunDirection = glm::vec3(-0.2f, -1.0f, 0.5f);
    glm::vec3 lightColor;
    glm::vec3 torchColor = glm::vec3(1.0f, 0.95f, 0.8f);

    Model suzanne("meshes/Ainz/ainz.obj");


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

        // Calculate Variables for this frame
        view = camera.GetViewMatrix();
        projection = glm::perspective(glm::radians(camera.Zoom), SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);


        // ==========================================
        // LIGHTS AND CAMERA VARIABLES
        //==========================================
        phongShader.use();

        // directional light (sun)
        glm::vec3 sunColor = glm::vec3(state.clearColor[0], state.clearColor[1], state.clearColor[2]);
        phongShader.setVec3("dirLight.direction", sunDirection);
        phongShader.setFloat("dirLight.intensity", 5.0f);
        phongShader.setVec3("dirLight.ambient", sunColor * 0.15f);
        phongShader.setVec3("dirLight.diffuse", sunColor * 1.0f);
        phongShader.setVec3("dirLight.specular", sunColor * 1.0f);

        // point lights

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

        // flashlight
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

        // camera matrices
        phongShader.setMat4("view", view);
        phongShader.setMat4("projection", projection);
        phongShader.setVec3("viewPos", camera.Position);

        // ==========================================
        // BEGIN DRAW
        // ==========================================
                
        // Clear Screen
        glClearColor(sunColor.r, sunColor.g, sunColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // ==========================================
        // DRAW CUBES
        // ==========================================
        // apply material parameters to current shader
        woodMaterial.apply();

        glBindVertexArray(VAO); 
        for(unsigned int i = 0; i < 7; i++)
        {   
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            model = glm::scale(model, glm::vec3(cubeScales[i])); 
            model = glm::rotate(model, cubeRotations[i], glm::vec3(0.0f, 1.0f, 0.0f));

            phongShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // ==========================================
        // DRAW FLOOR
        // ==========================================
        // swap to floor material!
        floorMaterial.apply();
        
        glm::mat4 floorModel = glm::mat4(1.0f); 
        //floorModel = glm::translate(floorModel, glm::vec3(0.0f, -2.0f, 0.0f));
        // use the same master phongShader
        phongShader.setMat4("model", floorModel); 

        glDrawArrays(GL_TRIANGLES, 36, 6);   

        // ==========================================
        // DRAW AINZ
        // ==========================================
        
        glm::mat4 ainzModel = glm::mat4(1.0f);
        ainzModel = glm::scale(ainzModel, glm::vec3(0.5f));
        ainzModel = glm::translate(ainzModel, glm::vec3(0.0f, 0.0f, 0.0f));
        ainzModel = glm::rotate(ainzModel, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        phongShader.setMat4("model", ainzModel);

        suzanne.draw(ainzMaterial);
        

        // ==========================================
        // DRAW LIGHT CUBE
        // ==========================================

        lightShader.use();
        lightShader.setMat4("view", view);
        lightShader.setMat4("projection", projection);

        glBindVertexArray(lightVAO); 

        for(unsigned int i = 0; i < 3; i++)
        {   
            lightColor = pointLightColors[i];
            lightShader.setVec3("Color", lightColor);
            glm::mat4 lightModel = glm::mat4(1.0f); 
            lightModel = glm::translate(lightModel, pointLightPositions[i]); 
            lightModel = glm::scale(lightModel, glm::vec3(0.2f)); 
            lightShader.setMat4("model", lightModel);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }



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