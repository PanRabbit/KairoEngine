#include <glad/glad.h> // Always load GLAD first to capture OpenGL pointers
#include <vector>
#include "UI.h"        
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "shader.h"

void InitUI(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark(); // Your favorite blue/black theme

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void RenderUI(UIVariables& state, const std::vector<Shader*>& engineShaders) {
    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Design the window layouts
    ImGui::Begin("Kairo Engine");
    
    ImGui::Text("Performance: %.1f FPS", ImGui::GetIO().Framerate);
    ImGui::Separator();

    // Modify state directly via references passed from main loop
    ImGui::Checkbox("Enable Wireframe Mode", &state.isWireframe);
    ImGui::ColorEdit3("Background Color", state.clearColor);

    if (ImGui::Button("Hot Reload Shaders")) {
        for (Shader* shader : engineShaders) {
            if (shader != nullptr) {
                shader->reload();
            }
        }
    }

    ImGui::End();

    // Draw the UI
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // Safety fix for EBO conflict
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ShutdownUI() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}