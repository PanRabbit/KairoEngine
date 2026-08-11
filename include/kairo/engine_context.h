#pragma once

#include "kairo/camera.h"
#include "kairo/selection.h"

struct EngineContext {
    Camera& camera;
    SelectionBuffer& selectionFB;
    Shader& selectionShader;
    std::vector<GameObject*>& sceneObjects;

    glm::mat4& view;       
    glm::mat4& projection; 

    float deltaTime = 0.0f;
    float scrWidth = 1600.0f;
    float scrHeight = 1200.0f;
    bool flashlightOn = false;
    int selectedObjectID = 0;

    // merged from UIVariables:
    bool isWireframe = false;
    float clearColor[3] = { 0.1f, 0.15f, 0.2f };
    bool reloadShader = false;
    float cameraSpeed = 3.0f;
    float fov = 45.0f;
    glm::vec3 cameraPos;
    glm::vec3 cameraRot;
};