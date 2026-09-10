#include <glad/glad.h> // Always load GLAD first to capture OpenGL pointers
#include "kairo/UI.h"        
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "kairo/shader.h"
#include "kairo/material.h"
#include <kairo/level_definition.h>
#include <ImGuizmo.h>

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

    // add imguizmo
    ImGuizmo::BeginFrame();
    ImGuizmo::SetOrthographic(false); // we're not using an orthographic camera
    ImGuizmo::SetRect(0, 0, engineContext.scrWidth, engineContext.scrHeight); // set the rect to the screen size
    const bool cameraLookActive = engineContext.flyCamLocked || engineContext.rmbLooking;
    ImGuizmo::Enable(!cameraLookActive); // don't steal mouse while looking around
    if (!cameraLookActive && engineContext.selectedObjectID != 0) {
        if (GameObject* obj = engineContext.getGameObjectByID(engineContext.selectedObjectID)) {
            static int gizmoObjectID = 0;
            static glm::mat4 gizmoMatrix(1.0f);
            if (obj->id != gizmoObjectID || !ImGuizmo::IsUsing()) {
                gizmoMatrix = obj->getTransformMatrix(); // rebuild from quat only when not dragging
                gizmoObjectID = obj->id;
            }
            ImGuizmo::Manipulate(
                glm::value_ptr(engineContext.view),
                glm::value_ptr(engineContext.projection),
                static_cast<ImGuizmo::OPERATION>(engineContext.transformOperation),
                static_cast<ImGuizmo::MODE>(engineContext.transformMode),
                glm::value_ptr(gizmoMatrix)
            );
            if (ImGuizmo::IsUsing()) {
                obj->setFromMatrix(gizmoMatrix); // write T/Q/S; no euler round-trip
            }
        }
    }



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
            ImGui::Checkbox("VSync", &engineContext.vsync);
            
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

        static int lastSelectedID = 0;
        const bool focusObjectTab = engineContext.selectedObjectID != 0 && engineContext.selectedObjectID != lastSelectedID; // flag on object change, but only for 1 cycle
        lastSelectedID = engineContext.selectedObjectID;
        if (ImGui::BeginTabItem("Objects", nullptr, focusObjectTab ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
        {
            // Transform Space Controls
            if (ImGui::RadioButton("World", engineContext.transformMode == ImGuizmo::WORLD)) {
                engineContext.transformMode = ImGuizmo::WORLD;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Local", engineContext.transformMode == ImGuizmo::LOCAL)) {
                engineContext.transformMode = ImGuizmo::LOCAL;
            }
            ImGui::SameLine();
            ImGui::Text("Transform Space: %s", engineContext.transformMode == ImGuizmo::WORLD ? "World" : "Local");

            // Transform Operation Controls (1/2/3 shortcuts, WASD stays free for camera)
            if (ImGui::RadioButton("Translate (1)", engineContext.transformOperation == ImGuizmo::TRANSLATE)) {
                engineContext.transformOperation = ImGuizmo::TRANSLATE;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Rotate (2)", engineContext.transformOperation == ImGuizmo::ROTATE)) {
                engineContext.transformOperation = ImGuizmo::ROTATE;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Scale (3)", engineContext.transformOperation == ImGuizmo::SCALE)) {
                engineContext.transformOperation = ImGuizmo::SCALE;
            }

            ImGui::Separator();
            if (engineContext.selectedObjectID != 0) {
                GameObject* selected = engineContext.getGameObjectByID(engineContext.selectedObjectID);
                ImGui::Text("Currently selected object: %s", selected->name.c_str());

                ImGui::DragFloat3("Position", &selected->position.x, 0.1f);
                glm::vec3 rotationDeg = glm::degrees(selected->getEulerXYZ());
                if (ImGui::DragFloat3("Rotation (deg)", &rotationDeg.x, 0.5f)) {
                    selected->setEulerXYZ(glm::radians(rotationDeg));
                }
                ImGui::DragFloat3("Scale", &selected->scale.x, 0.1f);
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