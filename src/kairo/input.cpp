#include <kairo/input.h>
#include <kairo/selection.h>
#include <kairo/camera.h>
#include <kairo/game_object.h>
#include <kairo/shader.h>
#include <iostream>

// Global context pointer for GLFW callbacks (set once at init)
static EngineContext* g_engineContext = nullptr;

void setInputContext(EngineContext* engineContext) {
    g_engineContext = engineContext;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// process input
void processInput(GLFWwindow *window, EngineContext& engineContext)
{   
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Camera Speed modifier (Sprint)
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        engineContext.camera.MovementSpeed = 2.5f;
    else
        engineContext.camera.MovementSpeed = 1.0f;

    // torch
    static bool fJustPressed = false;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    {
        if (!fJustPressed)
        {
            engineContext.flashlightOn = !engineContext.flashlightOn;
            fJustPressed = true;
        }
    }
    else
    {
        fJustPressed = false;
    }

    // Movement
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        engineContext.camera.ProcessKeyboard(FORWARD, engineContext.deltaTime);
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        engineContext.camera.ProcessKeyboard(BACKWARD, engineContext.deltaTime); 
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        engineContext.camera.ProcessKeyboard(LEFT, engineContext.deltaTime);
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        engineContext.camera.ProcessKeyboard(RIGHT, engineContext.deltaTime);
    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        engineContext.camera.ProcessKeyboard(UP, engineContext.deltaTime);
    if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        engineContext.camera.ProcessKeyboard(DOWN, engineContext.deltaTime);

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

            Shader* selectionShader = engineContext.getShaderByName("selection");
            if (selectionShader) {
                int selectedID = PerformSelection(mouseX, mouseY, static_cast<int>(engineContext.scrWidth), static_cast<int>(engineContext.scrHeight),
                                                *selectionShader, engineContext.selectionFB, engineContext.sceneObjects, engineContext.view, engineContext.projection);
                
                std::cout << "Clicked Object ID: " << selectedID << std::endl;
                engineContext.selectedObjectID = selectedID;
            }
            mouseJustPressed = true;
        }
    }
    else
    {
        mouseJustPressed = false;
    }
}

// Mouse state (moved from file-scope globals to per-context)
struct MouseState {
    float lastX = 0.0f;
    float lastY = 0.0f;
    bool firstMouse = true;
};

static MouseState g_mouseState;

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    auto& engineContext = *g_engineContext;

    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL)
    {
        g_mouseState.firstMouse = true;
        return;
    }

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (g_mouseState.firstMouse)
    {
        g_mouseState.lastX = xpos;
        g_mouseState.lastY = ypos;
        g_mouseState.firstMouse = false;
    }

    float xoffset = xpos - g_mouseState.lastX;
    float yoffset = g_mouseState.lastY - ypos; // reversed since y-coordinates go from bottom to top

    g_mouseState.lastX = xpos;
    g_mouseState.lastY = ypos;

    engineContext.camera.ProcessMouseMovement(xoffset, yoffset);
}

// scroll to change fov
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    auto& engineContext = *g_engineContext;
    engineContext.camera.ProcessMouseScroll(static_cast<float>(yoffset));
}