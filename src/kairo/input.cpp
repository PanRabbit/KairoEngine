#include <kairo/input.h>
#include <kairo/selection.h>
#include <kairo/camera.h>
#include <kairo/game_object.h>
#include <kairo/shader.h>
#include <iostream>
#include <imgui.h>
#include <ImGuizmo.h>

// Global context pointer for GLFW callbacks (set once at init)
static EngineContext* g_engineContext = nullptr;

void setInputContext(EngineContext* engineContext) {
    g_engineContext = engineContext;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Mouse state (moved from file-scope globals to per-context)
struct MouseState {
    float lastX = 0.0f;
    float lastY = 0.0f;
    bool firstMouse = true;
};

static MouseState g_mouseState;

// process input
void processInput(GLFWwindow *window, EngineContext& engineContext)
{   
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Camera Speed modifier (Sprint)
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        engineContext.camera.MovementSpeed = 5.0f;
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

    // Gizmo operation shortcuts (1/2/3)
    if (!ImGui::GetIO().WantTextInput)
    {
        static bool opKeyJustPressed = false;
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_1) == GLFW_PRESS)
        {
            if (!opKeyJustPressed)
            {
                engineContext.transformOperation = ImGuizmo::TRANSLATE;
                opKeyJustPressed = true;
            }
        }
        else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_2) == GLFW_PRESS)
        {
            if (!opKeyJustPressed)
            {
                engineContext.transformOperation = ImGuizmo::ROTATE;
                opKeyJustPressed = true;
            }
        }
        else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_3) == GLFW_PRESS)
        {
            if (!opKeyJustPressed)
            {
                engineContext.transformOperation = ImGuizmo::SCALE;
                opKeyJustPressed = true;
            }
        }
        else
        {
            opKeyJustPressed = false;
        }
    }

    // toggle mouse lock
    static bool tabJustPressed = false;
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) 
    {
        if (!tabJustPressed) 
        {
            engineContext.flyCamLocked = !engineContext.flyCamLocked;
            if (!engineContext.rmbLooking)
                glfwSetInputMode(window, GLFW_CURSOR, engineContext.flyCamLocked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
            tabJustPressed = true;
        }
    }
    else
    {
        tabJustPressed = false;
    }

    // Hold right-click to look around while the cursor is in UI mode
    const bool rmbHeld = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    if (rmbHeld && !engineContext.flyCamLocked)
    {
        if (!engineContext.rmbLooking && !ImGui::GetIO().WantCaptureMouse)
        {
            engineContext.rmbLooking = true;
            g_mouseState.firstMouse = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
    else if (engineContext.rmbLooking)
    {
        engineContext.rmbLooking = false;
        g_mouseState.firstMouse = true;
        if (!engineContext.flyCamLocked)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    // Mouse click selection (Only trigger when cursor is not captured by camera look)
    static bool mouseJustPressed = false;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if (!mouseJustPressed && glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL && !ImGui::GetIO().WantCaptureMouse && !ImGuizmo::IsOver() && !ImGuizmo::IsUsing()) // only trigger if not controlling camera, not in ImGui and not in ImGuizmo
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