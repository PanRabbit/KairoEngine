#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "UI.h"
#include "shader.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

float windowWidth = 1024;
float windowHeight = 1024;



// ====================================
// CAMERA VARIABLES SETUP
// ====================================

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f); // where the camera is
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); // where the camera is looking at
glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f); // world up vector


// we name this reverse direction because it's actually pointing in the opposite direction of the camera
glm::vec3 cameraReverseDirection= glm::normalize(cameraPos - cameraFront);
glm::vec3 cameraRight= glm::normalize(glm::cross(up, cameraReverseDirection)); // local right vector for the camera
glm::vec3 cameraUp= glm::cross(cameraReverseDirection, cameraRight); // local up vector for the camera


float cameraBaseSpeed = 1.0f;
float cameraTrueSpeed = 0.0f;

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
    cameraTrueSpeed = cameraBaseSpeed * deltaTime;

    // close engine
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // move forwards
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraTrueSpeed * cameraFront;

    // move backwards
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
       cameraPos -= cameraTrueSpeed * cameraFront;

    //move left
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
       cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraTrueSpeed;

    //move right
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
      cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraTrueSpeed;

    //move up
    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        cameraPos += cameraTrueSpeed * cameraUp;

    //move down
    if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        cameraPos -= cameraTrueSpeed * cameraUp;

    //change speed
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        cameraBaseSpeed = 2.5f;
    else
        cameraBaseSpeed = 1.0f;


    // toggle mouse lock - AI GENERATED - IMPROVE!!!
    static bool tabPressedLastFrame = false;

    bool tabPressedNow = (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS);

    if (tabPressedNow && !tabPressedLastFrame) // Key was just clicked
    {
        int currentMode = glfwGetInputMode(window, GLFW_CURSOR);
        glfwSetInputMode(window, GLFW_CURSOR, (currentMode == GLFW_CURSOR_NORMAL) ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }

    tabPressedLastFrame = tabPressedNow; // Remember the state for next time
    }

// handle mouse movement
float mouseLastX, mouseLastY;
float pitch, yaw = -90.0f;
bool firstMouse = true;

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL)
    {
        firstMouse = true;
        return;
    }
    if (firstMouse) // prevent the mouse from jumping when we first enter the window
    {
        mouseLastX = xpos;
        mouseLastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - mouseLastX;
    float yoffset = mouseLastY - ypos; // reversed since y-coordinates go from bottom to top
    mouseLastX = xpos;
    mouseLastY = ypos;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // lock camera's pitch to avoid flipping around
    if (pitch > 89.0f)
       pitch = 89.0f;
    if (pitch < -89.0f)
       pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

// scroll to zoom
float fov = 45.0f;
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
       fov = 1.0f;
    if (fov > 180.0f)
       fov = 180.0f;
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
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "KairoEngine", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    //set window states

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //lock mouse cursor to window

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSwapInterval(0); //vsync

    // init glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Set viewport uniform to match your 1024x1024 window so layout handles cleanly
    glViewport(0, 0, windowWidth, windowHeight);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glEnable(GL_DEPTH_TEST); //enable depth testing

    // ==========================================
    // SHADER COMPILATION & LINKING
    // ==========================================
  
    Shader cubeShader("shaders/vertex_shader.glsl", "shaders/cube_shader.glsl");
    Shader floorShader("shaders/vertex_shader.glsl", "shaders/floor_shader.glsl");

    std::vector<Shader*> allShaders = {&floorShader,  &cubeShader};

    // ==========================================
    // VERTEX DATA & BUFFERS (VAO & VBO)
    // ==========================================
    float vertices[] = {
        // positions          // colors           // texture coords
        // BACK FACE
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom-left
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom-right
         0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top-right
         0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top-right
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,   0.0f, 1.0f, // top-left
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom-left
    
        // FRONT FACE
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom-left
         0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom-right
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top-right
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top-right
        -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,   0.0f, 1.0f, // top-left
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom-left
    
        // LEFT FACE
        -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,   1.0f, 0.0f, // top-right
        -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top-left
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,   0.0f, 1.0f, // bottom-left
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,   0.0f, 1.0f, // bottom-left
        -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f, // bottom-right
        -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,   1.0f, 0.0f, // top-right
    
        // RIGHT FACE
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,   1.0f, 0.0f, // top-left
         0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top-right
         0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,   0.0f, 1.0f, // bottom-right
         0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,   0.0f, 1.0f, // bottom-right
         0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f, // bottom-left
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,   1.0f, 0.0f, // top-left
    
        // BOTTOM FACE
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,   0.0f, 1.0f, // top-right
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,   1.0f, 1.0f, // top-left
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // bottom-left
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // bottom-left
        -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,   0.0f, 0.0f, // bottom-right
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,   0.0f, 1.0f, // top-right
    
        // TOP FACE
        -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,   0.0f, 1.0f, // top-left
         0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,   1.0f, 1.0f, // top-right
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // bottom-right
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // bottom-right
        -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,   0.0f, 0.0f, // bottom-left
        -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,   0.0f, 1.0f,  // top-left

        //FLOOR
        // positions            // colors (gray)    // texture coords
        -10.0f, -1.0f, -10.0f,  0.5f, 0.5f, 0.5f,   0.0f,  0.0f, // bottom-left
        10.0f, -1.0f, -10.0f,  0.5f, 0.5f, 0.5f,   1.0f, 0.0f, // bottom-right
        10.0f, -1.0f,  10.0f,  0.5f, 0.5f, 0.5f,   1.0f, 1.0f,// top-right
        10.0f, -1.0f,  10.0f,  0.5f, 0.5f, 0.5f,   1.0f, 1.0f,// top-right
        -10.0f, -1.0f,  10.0f,  0.5f, 0.5f, 0.5f,   0.0f,  1.0f,// top-left
        -10.0f, -1.0f, -10.0f,  0.5f, 0.5f, 0.5f,   0.0f,  0.0f  // bottom-left
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



    // ==========================================
    // TEXTURES
    // ==========================================
    int width, height, nrChannels; // only declare once
    
    // ---- WOOD TEXTURE ----
    unsigned char *data_wood = stbi_load("textures/container.jpg", &width, &height, &nrChannels, 0);
    unsigned int texture_wood;
    glGenTextures(1, &texture_wood);
    glBindTexture(GL_TEXTURE_2D, texture_wood);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_wood);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data_wood);

    // ---- MIKU TEXTURE ----
    stbi_set_flip_vertically_on_load(true); // fixes image being upsidedown
    unsigned char *data_miku = stbi_load("textures/miku.png", &width, &height, &nrChannels, 0);
    unsigned int texture_miku;
    glGenTextures(1, &texture_miku);
    glBindTexture(GL_TEXTURE_2D, texture_miku);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_miku);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data_miku);






    // ==========================================
    // MATRICES & VECTORS
    // ==========================================

    // model matrix, transforms all objects vertices to world spcace. slightly rotated here to lay on the floor
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));


    // view matrix (basically the camera I think?) left blank to be set per frame
    glm::mat4 view;

    // projection matrix (make it not ortho), ,moved to render loop
    glm::mat4 projection;
    


    // ==========================================
    // OBJECTS
    // ==========================================

    glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f, 0.0f,
         0.0f),
        glm::vec3( 2.0f, 5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f, 3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f, 2.0f, -2.5f),
        glm::vec3( 1.5f, 0.2f, -1.5f),
        glm::vec3(-1.3f, 1.0f, -1.5f)
        };

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
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        projection = glm::perspective(glm::radians(fov), windowWidth / windowHeight, 0.1f, 100.0f);

        float r = sin(currentFrame) * 0.5f + 0.5f;
        float g = sin(currentFrame + 2.09439f) * 0.5f + 0.5f;
        float b = sin(currentFrame + 4.18879f) * 0.5f + 0.5f;


        // ==========================================
        // DRAW CUBES
        // ==========================================
        cubeShader.use();

        cubeShader.setMat4("view", view);
        cubeShader.setMat4("projection", projection);
        
        glUniform4f(glGetUniformLocation(cubeShader.ID, "col"), r, g, b, 1.0f);

        // Setup Textures
        cubeShader.setInt("sample1", 0);
        cubeShader.setInt("sample2", 1);
        glActiveTexture(GL_TEXTURE0); 
        glBindTexture(GL_TEXTURE_2D, texture_wood);
        glActiveTexture(GL_TEXTURE1); 
        glBindTexture(GL_TEXTURE_2D, texture_miku);
        
        glBindVertexArray(VAO); 

        // Draw cubes
        for(unsigned int i = 0; i < 10; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(currentFrame * (angle + 1)), glm::vec3(0.5f + angle, 1.0f + angle, 1.0f + angle));
            
            cubeShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }


        // ==========================================
        // DRAW FLOOR
        // ==========================================
        floorShader.use();

        floorShader.setMat4("view", view);
        floorShader.setMat4("projection", projection);
        glUniform4f(glGetUniformLocation(floorShader.ID, "col"), r, g, b, 1.0f);

        // We are already bound to VAO, so just update the model matrix
        glm::mat4 floorModel = glm::mat4(1.0f); 
        floorModel = glm::translate(floorModel, glm::vec3(0.0f, -2.0f, 0.0f));
        floorShader.setMat4("model", floorModel); 

        glDrawArrays(GL_TRIANGLES, 36, 6);       


        // ==========================================
        // END FRAME UI & SWAP
        // ==========================================
        state.cameraSpeed = cameraBaseSpeed;
        state.cameraPos = cameraPos;
        RenderUI(state, allShaders);

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