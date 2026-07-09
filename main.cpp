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
}

int main()
{
    // init glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // create window object
    GLFWwindow* window = glfwCreateWindow(1024, 1024, "KairoEngine", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwSwapInterval(0); //vsync

    // init glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Set viewport uniform to match your 1024x1024 window so layout handles cleanly
    glViewport(0, 0, 1024, 1024);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glEnable(GL_DEPTH_TEST); //enable depth testing

    // ==========================================
    // SHADER COMPILATION & LINKING
    // ==========================================
  
    Shader ourShader("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");

    std::vector<Shader*> allShaders = { &ourShader};

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
        -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,   0.0f, 1.0f  // top-left
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
    int width, height, nrChannels; // 1. Declared once here
    
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_miku);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data_miku);

    // ==========================================
    // MATRICES
    // ==========================================

    // model matrix, transforms all objects vertices to world spcace. slightly rotated here to lay on the floor
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));


    // view matrix (basically the camera I think?)
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

    // projection matrix (make it not ortho)
    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f), 1024.0f / 1024.0f, 0.1f, 100.0f);


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
        // process time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;


        processInput(window);

        // Process ui toggles toggle
        if (state.isWireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        
        // Clear screen dynamically using array values from our state object
        glClearColor(state.clearColor[0], state.clearColor[1], state.clearColor[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        //base matrices
        int modelLoc = glGetUniformLocation(ourShader.ID, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        int viewLoc = glGetUniformLocation(ourShader.ID, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        int projectionLoc = glGetUniformLocation(ourShader.ID, "projection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));


        int vertexColour = glGetUniformLocation(ourShader.ID, "col");

        float cycleTime = currentFrame;

        float r = sin(cycleTime) * 0.5f + 0.5f;
        float g = sin(cycleTime + 2.09439f) * 0.5f + 0.5f;
        float b = sin(cycleTime + 4.18879f) * 0.5f + 0.5f;

        glUniform4f(vertexColour, r, g, b, 1.0f);
        //std::cout << "r=" << r << std::endl;


        ourShader.use();

        //transformations
        model = glm::rotate(model, glm::radians(10.0f) * deltaTime, glm::vec3(0.5f, 1.0f, 1.0f));

        ourShader.setInt("sample1", 0); // Tells texture1 to read from GL_TEXTURE0
        ourShader.setInt("sample2", 1); // Tells texture2 to read from GL_TEXTURE1

        glActiveTexture(GL_TEXTURE0); 
        glBindTexture(GL_TEXTURE_2D, texture_wood);
        glActiveTexture(GL_TEXTURE1); 
        glBindTexture(GL_TEXTURE_2D, texture_miku);
        

        // Draw our object
        glBindVertexArray(VAO);  // rebind the VAO layout we created earlier
        //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); //rebind cause of nvidia driver bug

        for(unsigned int i = 0; i < 10; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(10.0f) * deltaTime, glm::vec3(0.5f, 1.0f, 1.0f));

            ourShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }




        // Render UI
        RenderUI(state, allShaders);

        // Poll events and swap buffers
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