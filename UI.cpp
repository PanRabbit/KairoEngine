#include <glad/glad.h> // Always load GLAD first to capture OpenGL pointers
#include <vector>
#include "UI.h"        
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "shader.h"

void InitUI(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsClassic();

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

    if (ImGui::BeginTabBar("MyTabBarID")) 
    {

        if (ImGui::BeginTabItem("General")) 
        {
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

            ImGui::Separator();
            ImGui::Text("Camera State");
            ImGui::DragFloat("Cam Base Speed", &state.cameraSpeed, 0.1f);
            ImGui::DragFloat3("Cam Position", &state.cameraPos.x, 0.1f);
            ImGui::DragFloat3("Cam Rotation", &state.cameraRot.x, 0.1f);
            ImGui::DragFloat("FOV", &state.fov, 0.1f);

            
            ImGui::EndTabItem(); 
        }

        if (ImGui::BeginTabItem("Objects"))
        {
            ImGui::Separator();
            ImGui::Text("Light");
            ImGui::Checkbox("Spin Light?", &state.lightSpin);

            if (state.lightSpin == true)
            {
                ImGui::SliderFloat("Spin Speed", &state.spinSpeed, 0.1f, 10.0f);
                ImGui::SliderFloat("Spin Radius", &state.spinRadius, 1.0f, 10.0f);
            }
            else 
            {
                ImGui::SliderFloat3("Light Position", &state.lightPos[0], -10.0f, 10.0f);
            }

            ImGui::Checkbox("Pulse Light?", &state.lightPulse);
             if (state.lightPulse == true)
             {
                ImGui::SliderFloat3("Pulse Speed", &state.pulseSpeed[0], 0.5f, 5.0f);
             }
             else
             {
                ImGui::ColorEdit3("Light Color", state.lightColor);
             }



            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("GUI Style Editor")) 
        {
            ImGui::Text("Make non-persistant style changes.");
            ImGui::ShowStyleEditor();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar(); 
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