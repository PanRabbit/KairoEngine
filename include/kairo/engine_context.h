#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <string>

#include "kairo/camera.h"
#include "kairo/shader.h"
#include "kairo/material.h"
#include "kairo/model.h"
#include "kairo/game_object.h"
#include "kairo/selection.h"
#include "kairo/texture.h"

struct EngineContext {
    
    Camera camera;                         
     
    SelectionBuffer selectionFB;             
    
    // owned maps for assets
    std::unordered_map<std::string, std::unique_ptr<Shader>> shaders;
    std::unordered_map<std::string, std::unique_ptr<Material>> materials;
    std::unordered_map<std::string, std::unique_ptr<Model>> models;
    std::unordered_map<std::string, std::unique_ptr<GameObject>> sceneObjects;

    // getters to access assets by name
    Shader* getShaderByName(const std::string& name);
    Material* getMaterialByName(const std::string& name);
    Model* getModelByName(const std::string& name);
    GameObject* getGameObjectByName(const std::string& name);
    GameObject* getGameObjectByID(int id);
    
    // skybox texture and VAO/VBO
    std::unique_ptr<CubeMapTexture> skyboxTexture;
    unsigned int skyboxVAO;
    unsigned int skyboxVBO;

    glm::mat4 view = glm::mat4(1.0f);       // view matrix for the camera
    glm::mat4 centerView = glm::mat4(1.0f); // view matrix for the center of the world
    glm::mat4 projection = glm::mat4(1.0f);  // projection matrix
    
    // Light data (render_loop needs this every frame) 
    std::vector<glm::vec3> pointLightPositions;
    std::vector<glm::vec3> pointLightColors;
    std::vector<float> pointLightIntensityMults;
    glm::vec3 sunDirection;
    glm::vec3 torchColor;

    // Level data 
    std::vector<glm::vec3> cubePositions;
    std::vector<float> cubeRotations;
    std::vector<float> cubeScales;

    std::vector<glm::vec3> grassPositions;
    
    // Timing states
    float deltaTime = 0.0f;
    static inline float lastFrame = 0.0f;     
    
    //  Post processing states 
    bool isPostProcessing = true;
    unsigned int postProcessingFB;
    unsigned int texColorBuffer;
    unsigned int rboDepthStencil;
    unsigned int PPVBO;
    unsigned int PPVAO;
    unsigned int intermediateFBO;
    unsigned int intermediateTex;

    // UI states 
    float scrWidth = 1600.0f;
    float scrHeight = 1200.0f;
    bool flashlightOn = false;
    int selectedObjectID = 0;
    
    bool isWireframe = false;
    glm::vec3 clearColor = glm::vec3(0.1f, 0.15f, 0.2f);
    bool reloadShader = false;
    float cameraSpeed = 3.0f;
    float fov = 45.0f;
    glm::vec3 cameraPos;
    glm::vec3 cameraRot;
};
