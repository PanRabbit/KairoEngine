#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>


// define vertex shader
const char *vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
" gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";

// define fragment shader
const char *fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
" FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\0";


// handle window resize
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// process input. this is where we'll keep the various controls and inputs for the program.
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
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // create window object and check if it was created successfully
    GLFWwindow* window = glfwCreateWindow(800, 600, "KairoEngine", NULL, NULL);
    if (window == NULL)
    {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
    }
    // make the window the current context
    glfwMakeContextCurrent(window);

    // init glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
    }

    // set viewport
    glViewport(0, 0, 800, 600);


    // set callback for window resize
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


    // set up vertex data (and buffer(s)) and configure vertex attributes

    // define vertices for a triangle
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f, 0.5f, 0.0f
        };


    // set up vertex buffer object
    unsigned int VBO;
    // generate 1 buffer object and store the ID in VBO
    glGenBuffers(1, &VBO);
    // bind the buffer object to the GL_ARRAY_BUFFER target
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // copy the vertex data to the buffer object
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // set up and compile vertex shader
    unsigned int vertexShader;
    // create a vertex shader object
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    // attach the shader source code to the shader object
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    // compile the shader
    glCompileShader(vertexShader);
    // check for compilation errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }





    //=========RENDER LOOP=========

    // render loop. stays true until the window is asked to close.
    while(!glfwWindowShouldClose(window))
    {

    //check for input and process it.
    processInput(window);

    // render here
    glClearColor(0.1f, 0.4f, 0.6f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);




    // poll events. this checks for any pending events and calls the corresponding functions.
    glfwPollEvents();

    // swap buffers. this swaps the front and back buffers of the window. This is called double buffering and is used to prevent flickering by 
    // rendering the frame to the back buffer and then swapping it with the front buffer so the incomplete frame is not displayed to the user.
    glfwSwapBuffers(window);
    }
    //=========END RENDER LOOP=========


    // terminate glfw, free resources, and exit the program.
    glfwTerminate();
    return 0;
}

