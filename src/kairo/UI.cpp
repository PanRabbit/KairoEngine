#include <glad/glad.h> // Always load GLAD first to capture OpenGL pointers
#include "kairo/UI.h"        
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "kairo/shader.h"
#include "kairo/material.h"
#include <kairo/level_definition.h>

void InitUI(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsClassic();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void RenderUI(EngineContext& engineContext) {
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

            // Modify engineContext directly via references passed from main loop
            ImGui::Checkbox("Enable Wireframe Mode", &engineContext.isWireframe);
            ImGui::Checkbox("Enable Post Processing", &engineContext.isPostProcessing);
            
            float clearColor[3] = { engineContext.clearColor.x, engineContext.clearColor.y, engineContext.clearColor.z };
            ImGui::ColorEdit3("Background Color", clearColor);
            engineContext.clearColor = glm::vec3(clearColor[0], clearColor[1], clearColor[2]);

            if (ImGui::Button("Hot Reload Shaders")) {
                for (auto& [name, shader] : engineContext.shaders) {
                    shader->reload();
                }
                for (auto& [name, material] : engineContext.materials) {
                    material->loadFromJson("reload");
                }
                engineContext.reloadShader = true;
            }
            ImGui::Separator();
            ImGui::Text("Environments:");
            if (ImGui::Button("Day")) {
                LoadLevelFromJson(engineContext, "levels/day.json");
            }
            if (ImGui::Button("Night")) {
                LoadLevelFromJson(engineContext, "levels/night.json");
            }
            if (ImGui::Button("Cloudy")) {
                LoadLevelFromJson(engineContext, "levels/cloudy.json");
            }

            ImGui::Separator();
            ImGui::Text("Camera engineContext");
            ImGui::DragFloat("Cam Base Speed", &engineContext.cameraSpeed, 0.1f);
            ImGui::DragFloat3("Cam Position", &engineContext.cameraPos.x, 0.1f);
            ImGui::DragFloat3("Cam Rotation", &engineContext.cameraRot.x, 0.1f);
            ImGui::DragFloat("FOV", &engineContext.fov, 0.1f);

            
            ImGui::EndTabItem(); 
        }

        if (ImGui::BeginTabItem("Objects"))
        {
            ImGui::Separator();
            if (engineContext.selectedObjectID != 0) {
                ImGui::Text("Currently selected object: %s", engineContext.getGameObjectByID(engineContext.selectedObjectID)->name.c_str());

                ImGui::DragFloat3("Position", &engineContext.getGameObjectByID(engineContext.selectedObjectID)->position.x, 0.1f);
                ImGui::DragFloat3("Rotation", &engineContext.getGameObjectByID(engineContext.selectedObjectID)->rotation.x, 0.1f);
                ImGui::DragFloat3("Scale", &engineContext.getGameObjectByID(engineContext.selectedObjectID)->scale.x, 0.1f);
            } else {
                ImGui::Text("No object selected");
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