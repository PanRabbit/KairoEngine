#include <glad/glad.h> // Always load GLAD first to capture OpenGL pointers
#include "kairo/UI.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "kairo/shader.h"
#include "kairo/material.h"
#include <kairo/level_definition.h>
#include <kairo/skybox.h>
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

static char g_saveAsName[128] = "";
static std::string g_overwritePath;

static std::string LevelDisplayName(const std::string& path)
{
    return std::filesystem::path(path).stem().string();
}

static bool OverwriteConfirmPopup(const char* popupId, EngineContext& engineContext)
{
    bool saved = false;
    if (ImGui::BeginPopupModal(popupId, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Overwrite \"%s\"?", LevelDisplayName(g_overwritePath).c_str());
        ImGui::TextUnformatted("This cannot be undone.");
        if (ImGui::Button("Overwrite", ImVec2(120, 0))) {
            SaveLevelToJson(engineContext, g_overwritePath);
            saved = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    return saved;
}

static void SaveAsPopup(EngineContext& engineContext)
{
    if (ImGui::BeginPopupModal("Save Level As", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Name", g_saveAsName, sizeof(g_saveAsName));
        if (ImGui::Button("Save") && g_saveAsName[0] != '\0') {
            const std::string path = MakeLevelPath(g_saveAsName);
            if (!path.empty() && std::filesystem::exists(path)) {
                g_overwritePath = path;
                ImGui::OpenPopup("Overwrite Level As?");
            } else if (SaveLevelAs(engineContext, g_saveAsName)) {
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();

        if (OverwriteConfirmPopup("Overwrite Level As?", engineContext))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }
}

void InitUI(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsClassic();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

static void PostProcessControls(EngineContext& engineContext)
{
    ImGui::Checkbox("Enable Post Processing", &engineContext.isPostProcessing);
    ImGui::BeginDisabled(!engineContext.isPostProcessing);

    ImGui::SeparatorText("Tonemap");
    ImGui::DragFloat("Exposure", &engineContext.exposure, 0.01f, 0.01f, 10.0f);

    ImGui::SeparatorText("Bloom");
    ImGui::Checkbox("Enable Bloom", &engineContext.enableBloom);
    ImGui::BeginDisabled(!engineContext.enableBloom);
    ImGui::DragFloat("Threshold", &engineContext.bloomThreshold, 0.01f, 0.0f, 8.0f);
    ImGui::DragFloat("Blur Radius", &engineContext.bloomBlurRadius, 0.01f, 0.01f, 10.0f);
    ImGui::DragFloat("Intensity", &engineContext.bloomIntensity, 0.01f, 0.0f, 8.0f);
    ImGui::EndDisabled();

    ImGui::SeparatorText("Sharpen");
    ImGui::Checkbox("Enable Sharpen", &engineContext.enableSharpen);
    ImGui::BeginDisabled(!engineContext.enableSharpen);
    ImGui::DragFloat("Sharpness", &engineContext.sharpness, 0.05f, 0.0f, 16.0f);
    ImGui::EndDisabled();

    ImGui::SeparatorText("Blur");
    ImGui::Checkbox("Enable Blur", &engineContext.enableBlur);
    ImGui::BeginDisabled(!engineContext.enableBlur);
    ImGui::DragFloat("Blur Strength", &engineContext.blurStrength, 0.05f, 0.0f, 16.0f);
    ImGui::EndDisabled();

    ImGui::SeparatorText("Edge Detection");
    ImGui::Checkbox("Enable Edge Detection", &engineContext.enableEdgeDetection);
    ImGui::BeginDisabled(!engineContext.enableEdgeDetection);
    ImGui::DragFloat("Edge Strength", &engineContext.edgeDetectionStrength, 0.05f, 0.0f, 16.0f);
    ImGui::EndDisabled();

    ImGui::SeparatorText("Pixelate");
    ImGui::Checkbox("Enable Pixelate", &engineContext.enablePixelate);
    ImGui::BeginDisabled(!engineContext.enablePixelate);
    ImGui::DragFloat("Resolution", &engineContext.pixelateResolution, 1.0f, 8.0f, 1080.0f);
    ImGui::EndDisabled();

    ImGui::EndDisabled();
}

static glm::mat4 MakeLightGizmoMatrix(const glm::vec3& position, const glm::vec3& direction, bool oriented)
{
    if (!oriented)
        return glm::translate(glm::mat4(1.0f), position);

    glm::vec3 forward = (glm::dot(direction, direction) < 1e-8f)
        ? glm::vec3(0.0f, -1.0f, 0.0f)
        : glm::normalize(direction);
    glm::vec3 up = (glm::abs(forward.y) > 0.99f) ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::normalize(glm::cross(up, forward));
    glm::vec3 localUp = glm::cross(forward, right);

    glm::mat4 matrix(1.0f);
    matrix[0] = glm::vec4(right, 0.0f);
    matrix[1] = glm::vec4(localUp, 0.0f);
    matrix[2] = glm::vec4(-forward, 0.0f);
    matrix[3] = glm::vec4(position, 1.0f);
    return matrix;
}

static void ExtractLightGizmoMatrix(const glm::mat4& matrix, glm::vec3& position, glm::vec3* direction)
{
    position = glm::vec3(matrix[3]);
    if (direction)
        *direction = glm::normalize(-glm::vec3(matrix[2]));
}

static bool ComboStringList(const char* label, const std::string& current, const std::vector<std::string>& items, std::string& outSelected)
{
    bool changed = false;
    const char* preview = current.empty() ? "(none)" : current.c_str();
    if (ImGui::BeginCombo(label, preview)) {
        for (const std::string& item : items) {
            const bool selected = (item == current);
            if (ImGui::Selectable(item.c_str(), selected)) {
                outSelected = item;
                changed = true;
            }
            if (selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    return changed;
}

static void OpenSaveAsPopup(const EngineContext& engineContext)
{
    std::string stem = engineContext.currentLevelPath.empty()
        ? "untitled"
        : LevelDisplayName(engineContext.currentLevelPath);
    std::snprintf(g_saveAsName, sizeof(g_saveAsName), "%s", stem.c_str());
    ImGui::OpenPopup("Save Level As");
}

static void ManipulateSelectedGizmo(EngineContext& engineContext)
{
    const int id = engineContext.selectedObjectID;
    if (id == 0)
        return;

    static int gizmoObjectID = 0;
    static glm::mat4 gizmoMatrix(1.0f);
    static float gizmoBaseRadius = 1.0f;

    ImGuizmo::OPERATION operation = static_cast<ImGuizmo::OPERATION>(engineContext.transformOperation);

    if (id == EngineContext::SUN_SELECT_ID) {
        operation = ImGuizmo::ROTATE;
        if (id != gizmoObjectID || !ImGuizmo::IsUsing()) {
            gizmoMatrix = MakeLightGizmoMatrix(EngineContext::SUN_GIZMO_ORIGIN, engineContext.sunDirection, true);
            gizmoObjectID = id;
        }
        ImGuizmo::Manipulate(
            glm::value_ptr(engineContext.view),
            glm::value_ptr(engineContext.projection),
            operation,
            static_cast<ImGuizmo::MODE>(engineContext.transformMode),
            glm::value_ptr(gizmoMatrix)
        );
        if (ImGuizmo::IsUsing()) {
            glm::vec3 handlePos;
            glm::vec3 handleDir;
            ExtractLightGizmoMatrix(gizmoMatrix, handlePos, &handleDir);
            engineContext.sunDirection = handleDir;
            gizmoMatrix[3] = glm::vec4(EngineContext::SUN_GIZMO_ORIGIN, 1.0f);
        }
        return;
    }

    if (GameObject* obj = engineContext.getGameObjectByID(id)) {
        if (obj->id != gizmoObjectID || !ImGuizmo::IsUsing()) {
            gizmoMatrix = obj->getTransformMatrix();
            gizmoObjectID = obj->id;
        }
        ImGuizmo::Manipulate(
            glm::value_ptr(engineContext.view),
            glm::value_ptr(engineContext.projection),
            operation,
            static_cast<ImGuizmo::MODE>(engineContext.transformMode),
            glm::value_ptr(gizmoMatrix)
        );
        if (ImGuizmo::IsUsing())
            obj->setFromMatrix(gizmoMatrix);
        return;
    }

    glm::vec3* positionPtr = nullptr;
    glm::vec3* directionPtr = nullptr;
    float* radiusPtr = nullptr;
    bool oriented = false;

    if (EngineContext::IsPointLightSelectID(id)) {
        const int index = EngineContext::PointLightIndexFromSelectID(id);
        if (index < 0 || index >= static_cast<int>(engineContext.pointLightPositions.size()))
            return;
        positionPtr = &engineContext.pointLightPositions[index];
        radiusPtr = &engineContext.pointLightRadii[index];
        if (operation == ImGuizmo::ROTATE)
            operation = ImGuizmo::TRANSLATE;
    } else if (EngineContext::IsSpotLightSelectID(id)) {
        const int index = EngineContext::SpotLightIndexFromSelectID(id);
        if (index < 1 || index >= static_cast<int>(engineContext.spotLightPositions.size()))
            return;
        positionPtr = &engineContext.spotLightPositions[index];
        directionPtr = &engineContext.spotLightDirections[index];
        radiusPtr = &engineContext.spotLightRadii[index];
        oriented = true;
    } else {
        return;
    }

    if (id != gizmoObjectID || !ImGuizmo::IsUsing()) {
        gizmoMatrix = MakeLightGizmoMatrix(*positionPtr, directionPtr ? *directionPtr : glm::vec3(0.0f, -1.0f, 0.0f), oriented);
        gizmoBaseRadius = *radiusPtr;
        gizmoObjectID = id;
    }

    ImGuizmo::Manipulate(
        glm::value_ptr(engineContext.view),
        glm::value_ptr(engineContext.projection),
        operation,
        static_cast<ImGuizmo::MODE>(engineContext.transformMode),
        glm::value_ptr(gizmoMatrix)
    );
    if (ImGuizmo::IsUsing()) {
        ExtractLightGizmoMatrix(gizmoMatrix, *positionPtr, directionPtr);
        if (operation == ImGuizmo::SCALE)
            *radiusPtr = glm::max(0.1f, gizmoBaseRadius * glm::length(glm::vec3(gizmoMatrix[0])));
    }
}

static void LevelEditorUI(EngineContext& engineContext)
{
    ImGui::SeparatorText("Level");

    const std::vector<std::string> levels = ListLevelFiles();
    const std::string currentLevel = engineContext.currentLevelPath.empty()
        ? "(unsaved)"
        : LevelDisplayName(engineContext.currentLevelPath);
    if (ImGui::BeginCombo("Level", currentLevel.c_str())) {
        for (const std::string& path : levels) {
            const bool selected = (path == engineContext.currentLevelPath);
            if (ImGui::Selectable(LevelDisplayName(path).c_str(), selected))
                LoadLevelFromJson(engineContext, path);
            if (selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    if (ImGui::Button("Save Level")) {
        if (!engineContext.currentLevelPath.empty()) {
            g_overwritePath = engineContext.currentLevelPath;
            ImGui::OpenPopup("Overwrite Level?");
        } else
            OpenSaveAsPopup(engineContext);
    }
    ImGui::SameLine();
    if (ImGui::Button("Save As New"))
        OpenSaveAsPopup(engineContext);SaveAsPopup(engineContext);
    OverwriteConfirmPopup("Overwrite Level?", engineContext);

    const std::vector<std::string> skyboxes = ListSkyboxNames();
    std::string chosenSkybox;
    if (ComboStringList("Skybox", engineContext.currentSkyboxName, skyboxes, chosenSkybox))
        DefineSkyBox(engineContext, chosenSkybox);

    ImGui::SeparatorText("Add to Level");
    const float listHeight = ImGui::GetTextLineHeightWithSpacing() * 7.0f;
    const float columnWidth = ImGui::GetContentRegionAvail().x * 0.5f - 4.0f;

    ImGui::BeginChild("AddObjects", ImVec2(columnWidth, listHeight), true);
    ImGui::TextUnformatted("Objects");
    ImGui::Separator();
    if (ImGui::BeginListBox("##AddObjectList", ImVec2(-1, -1))) {
        for (const std::string& modelName : ListModelNames(engineContext)) {
            ImGui::PushID(modelName.c_str());
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(modelName.c_str());
            ImGui::SameLine();
            if (ImGui::SmallButton("+"))
                AddObjectToLevel(engineContext, modelName);
            ImGui::PopID();
        }
        ImGui::EndListBox();
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("AddLights", ImVec2(columnWidth, listHeight), true);
    ImGui::TextUnformatted("Lights");
    ImGui::Separator();
    if (ImGui::Button("Point Light", ImVec2(-1, 0)))
        AddPointLightToLevel(engineContext);
    if (ImGui::Button("Spot Light", ImVec2(-1, 0)))
        AddSpotLightToLevel(engineContext);
    if (ImGui::Button(engineContext.showLightSpheres ? "Hide Light Spheres" : "Show Light Spheres", ImVec2(-1, 0)))
        engineContext.showLightSpheres = !engineContext.showLightSpheres;
    ImGui::EndChild();

    ImGui::SeparatorText("Scene");
    if (ImGui::BeginListBox("##SceneList", ImVec2(-1, ImGui::GetTextLineHeightWithSpacing() * 10.0f))) {
        std::vector<std::string> objectNames;
        objectNames.reserve(engineContext.sceneObjects.size());
        for (const auto& [name, _] : engineContext.sceneObjects)
            objectNames.push_back(name);
        std::sort(objectNames.begin(), objectNames.end());
        for (const std::string& name : objectNames) {
            GameObject* obj = engineContext.sceneObjects[name].get();
            const bool selected = (engineContext.selectedObjectID == obj->id);
            if (ImGui::Selectable(name.c_str(), selected))
                engineContext.selectedObjectID = obj->id;
        }

        ImGui::Separator();
        if (ImGui::Selectable("Sun", engineContext.selectedObjectID == EngineContext::SUN_SELECT_ID))
            engineContext.selectedObjectID = EngineContext::SUN_SELECT_ID;

        for (int i = 0; i < static_cast<int>(engineContext.pointLightPositions.size()); ++i) {
            const std::string label = "Point Light " + std::to_string(i);
            const int selectID = EngineContext::PointLightSelectID(i);
            if (ImGui::Selectable(label.c_str(), engineContext.selectedObjectID == selectID))
                engineContext.selectedObjectID = selectID;
        }
        for (int i = 1; i < static_cast<int>(engineContext.spotLightPositions.size()); ++i) {
            const std::string label = "Spot Light " + std::to_string(i - 1);
            const int selectID = EngineContext::SpotLightSelectID(i);
            if (ImGui::Selectable(label.c_str(), engineContext.selectedObjectID == selectID))
                engineContext.selectedObjectID = selectID;
        }
        ImGui::EndListBox();
    }
}

static void SelectionInspectorUI(EngineContext& engineContext)
{
    ImGui::SeparatorText("Selection");

    if (ImGui::RadioButton("World", engineContext.transformMode == ImGuizmo::WORLD))
        engineContext.transformMode = ImGuizmo::WORLD;
    ImGui::SameLine();
    if (ImGui::RadioButton("Local", engineContext.transformMode == ImGuizmo::LOCAL))
        engineContext.transformMode = ImGuizmo::LOCAL;

    if (ImGui::RadioButton("Translate (1)", engineContext.transformOperation == ImGuizmo::TRANSLATE))
        engineContext.transformOperation = ImGuizmo::TRANSLATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Rotate (2)", engineContext.transformOperation == ImGuizmo::ROTATE))
        engineContext.transformOperation = ImGuizmo::ROTATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Scale (3)", engineContext.transformOperation == ImGuizmo::SCALE))
        engineContext.transformOperation = ImGuizmo::SCALE;

    ImGui::Separator();

    const int id = engineContext.selectedObjectID;
    if (id == 0) {
        ImGui::TextUnformatted("No object selected");
        return;
    }

    if (id == EngineContext::SUN_SELECT_ID) {
        ImGui::TextUnformatted("Sun (directional light)");
        ImGui::DragFloat3("Direction", &engineContext.sunDirection.x, 0.01f);
        float sunColor[3] = { engineContext.clearColor.x, engineContext.clearColor.y, engineContext.clearColor.z };
        if (ImGui::ColorEdit3("Color", sunColor))
            engineContext.clearColor = glm::vec3(sunColor[0], sunColor[1], sunColor[2]);
        return;
    }

    if (EngineContext::IsPointLightSelectID(id)) {
        const int index = EngineContext::PointLightIndexFromSelectID(id);
        if (index < 0 || index >= static_cast<int>(engineContext.pointLightPositions.size())) {
            ImGui::TextUnformatted("No object selected");
            return;
        }
        ImGui::Text("Point Light %d", index);
        ImGui::DragFloat3("Position", &engineContext.pointLightPositions[index].x, 0.1f);
        float color[3] = {
            engineContext.pointLightColors[index].x,
            engineContext.pointLightColors[index].y,
            engineContext.pointLightColors[index].z
        };
        if (ImGui::ColorEdit3("Color", color))
            engineContext.pointLightColors[index] = glm::vec3(color[0], color[1], color[2]);
        ImGui::DragFloat("Intensity", &engineContext.pointLightIntensityMults[index], 0.1f, 0.0f, 64.0f);
        ImGui::DragFloat("Radius", &engineContext.pointLightRadii[index], 0.1f, 0.1f, 128.0f);
        if (ImGui::Button("Duplicate"))
            DuplicateSelectedObject(engineContext);
        ImGui::SameLine();
        if (ImGui::Button("Delete"))
            DeleteSelectedFromLevel(engineContext);
        return;
    }

    if (EngineContext::IsSpotLightSelectID(id)) {
        const int index = EngineContext::SpotLightIndexFromSelectID(id);
        if (index < 1 || index >= static_cast<int>(engineContext.spotLightPositions.size())) {
            ImGui::TextUnformatted("No object selected");
            return;
        }
        ImGui::Text("Spot Light %d", index - 1);
        ImGui::DragFloat3("Position", &engineContext.spotLightPositions[index].x, 0.1f);
        if (ImGui::DragFloat3("Direction", &engineContext.spotLightDirections[index].x, 0.01f)) {
            glm::vec3& dir = engineContext.spotLightDirections[index];
            if (glm::dot(dir, dir) > 1e-8f)
                dir = glm::normalize(dir);
        }
        float color[3] = {
            engineContext.spotLightColors[index].x,
            engineContext.spotLightColors[index].y,
            engineContext.spotLightColors[index].z
        };
        if (ImGui::ColorEdit3("Color", color))
            engineContext.spotLightColors[index] = glm::vec3(color[0], color[1], color[2]);
        ImGui::DragFloat("Intensity", &engineContext.spotLightIntensityMults[index], 0.1f, 0.0f, 64.0f);
        ImGui::DragFloat("Inner Cutoff", &engineContext.spotLightCutOffs[index], 0.1f, 0.0f, 89.0f);
        ImGui::DragFloat("Outer Cutoff", &engineContext.spotLightOuterCutOffs[index], 0.1f, 0.0f, 89.0f);
        ImGui::DragFloat("Radius", &engineContext.spotLightRadii[index], 0.1f, 0.1f, 128.0f);
        if (ImGui::Button("Duplicate"))
            DuplicateSelectedObject(engineContext);
        ImGui::SameLine();
        if (ImGui::Button("Delete"))
            DeleteSelectedFromLevel(engineContext);
        return;
    }

    GameObject* selected = engineContext.getGameObjectByID(id);
    if (!selected) {
        ImGui::TextUnformatted("No object selected");
        return;
    }

    ImGui::Text("Object: %s", selected->name.c_str());
    ImGui::Text("Model: %s", selected->modelName.c_str());

    std::string chosenMaterial;
    if (ComboStringList("Material", selected->materialName, ListMaterialNames(engineContext), chosenMaterial)) {
        selected->material = engineContext.getMaterialByName(chosenMaterial);
        selected->materialName = chosenMaterial;
    }

    ImGui::DragFloat3("Position", &selected->position.x, 0.1f);
    glm::vec3 rotationDeg = glm::degrees(selected->getEulerXYZ());
    if (ImGui::DragFloat3("Rotation (deg)", &rotationDeg.x, 0.5f))
        selected->setEulerXYZ(glm::radians(rotationDeg));
    ImGui::DragFloat3("Scale", &selected->scale.x, 0.1f);
    if (ImGui::Button("Duplicate"))
        DuplicateSelectedObject(engineContext);
    ImGui::SameLine();
    if (ImGui::Button("Delete"))
        DeleteSelectedFromLevel(engineContext);
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
    if (!cameraLookActive)
        ManipulateSelectedGizmo(engineContext);

    // Design the window layouts
    ImGui::Begin("Kairo Engine");

    if (ImGui::BeginTabBar("MyTabBarID"))
    {
        if (ImGui::BeginTabItem("General"))
        {
            ImGui::Text("Performance: %.1f FPS", ImGui::GetIO().Framerate);
            ImGui::Separator();

            ImGui::Checkbox("Enable Wireframe Mode", &engineContext.isWireframe);
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
            ImGui::Text("Camera engineContext");
            ImGui::DragFloat("Cam Base Speed", &engineContext.cameraSpeed, 0.1f);
            ImGui::DragFloat3("Cam Position", &engineContext.cameraPos.x, 0.1f);
            ImGui::DragFloat3("Cam Rotation", &engineContext.cameraRot.x, 0.1f);
            ImGui::DragFloat("FOV", &engineContext.fov, 0.1f);

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Post Processing"))
        {
            PostProcessControls(engineContext);
            ImGui::EndTabItem();
        }

        static int lastSelectedID = 0;
        const bool focusObjectTab = engineContext.selectedObjectID != 0 && engineContext.selectedObjectID != lastSelectedID;
        lastSelectedID = engineContext.selectedObjectID;
        if (ImGui::BeginTabItem("Objects", nullptr, focusObjectTab ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
        {
            LevelEditorUI(engineContext);
            SelectionInspectorUI(engineContext);
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
