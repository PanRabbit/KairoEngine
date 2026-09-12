#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
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
    static constexpr int MAX_POINT_LIGHTS = 32;
    static constexpr int MAX_SPOT_LIGHTS = 32;
    static constexpr int POINT_SHADOW_TEXTURE_UNIT = 16;
    static constexpr int SPOT_SHADOW_TEXTURE_UNIT = 48;
    static constexpr int SUN_SHADOW_TEXTURE_UNIT = 99;

    static constexpr float FLASHLIGHT_INTENSITY = 32.0f;
    static constexpr float FLASHLIGHT_RADIUS = 64.0f;
    static constexpr float FLASHLIGHT_CUT_OFF = 12.5f;
    static constexpr float FLASHLIGHT_OUTER_CUT_OFF = 20.0f;
    static constexpr float DEFAULT_POINT_LIGHT_RADIUS = 8.0f;
    static constexpr float DEFAULT_SPOT_LIGHT_RADIUS = 16.0f;
    static constexpr float DEFAULT_SPOT_CUT_OFF = 20.0f;
    static constexpr float DEFAULT_SPOT_OUTER_CUT_OFF = 28.0f;

    static constexpr int POINT_LIGHT_SELECT_BASE = 100000;
    static constexpr int SPOT_LIGHT_SELECT_BASE = 200000;
    static constexpr int SUN_SELECT_ID = 300000;
    static constexpr glm::vec3 SUN_GIZMO_ORIGIN = glm::vec3(0.0f, 0.0f, 0.0f);

    static int PointLightSelectID(int index) { return POINT_LIGHT_SELECT_BASE + index; }
    static int SpotLightSelectID(int index) { return SPOT_LIGHT_SELECT_BASE + index; }
    static bool IsPointLightSelectID(int id) {
        return id >= POINT_LIGHT_SELECT_BASE && id < SPOT_LIGHT_SELECT_BASE;
    }
    static bool IsSpotLightSelectID(int id) {
        return id >= SPOT_LIGHT_SELECT_BASE && id < SUN_SELECT_ID;
    }
    static int PointLightIndexFromSelectID(int id) { return id - POINT_LIGHT_SELECT_BASE; }
    static int SpotLightIndexFromSelectID(int id) { return id - SPOT_LIGHT_SELECT_BASE; }

    glm::vec3 sunHandlePosition() const {
        return SUN_GIZMO_ORIGIN;
    }
    
    Camera camera;                         
     
    SelectionBuffer selectionFB;             
    
    // owned maps for assets
    std::unordered_map<std::string, std::unique_ptr<Shader>> shaders;
    std::unordered_map<std::string, std::unique_ptr<Material>> materials;
    std::unordered_map<std::string, std::unique_ptr<Model>> models;
    std::unordered_map<std::string, std::unique_ptr<GameObject>> sceneObjects;
    std::unordered_map<std::string, std::string> modelFolders;
    // getters to access assets by name
    Shader* getShaderByName(const std::string& name);
    Material* getMaterialByName(const std::string& name);
    Model* getModelByName(const std::string& name);
    GameObject* getGameObjectByName(const std::string& name);
    GameObject* getGameObjectByID(int id);
    
    // skybox texture and VAO/VBO
    std::unique_ptr<CubeMapTexture> skyboxTexture;
    unsigned int skyboxVAO = 0;
    unsigned int skyboxVBO = 0;
    std::string currentSkyboxName;
    std::string currentLevelPath;

    glm::mat4 view = glm::mat4(1.0f);       // view matrix for the camera
    glm::mat4 centerView = glm::mat4(1.0f); // view matrix for the center of the world
    glm::mat4 projection = glm::mat4(1.0f);  // projection matrix
    
    // Light data (render_loop needs this every frame) 
    std::vector<glm::vec3> pointLightPositions;
    std::vector<glm::vec3> pointLightColors;
    std::vector<float> pointLightIntensityMults;
    std::vector<float> pointLightRadii;

    std::vector<glm::vec3> spotLightPositions;
    std::vector<glm::vec3> spotLightDirections;
    std::vector<glm::vec3> spotLightColors;
    std::vector<float> spotLightIntensityMults;
    std::vector<float> spotLightCutOffs;      // inner cone, degrees
    std::vector<float> spotLightOuterCutOffs; // outer cone, degrees
    std::vector<float> spotLightRadii;

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

    // Shadow mapping states
    unsigned int shadowDepthMapFBO = 0;
    unsigned int shadowDepthMapTexture = 0;
    glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);

    std::vector<unsigned int> pointLightShadowCubemaps; // one cubemap per point light
    std::vector<unsigned int> pointLightShadowFBOs; // one FBO per point light
    std::vector<std::array<glm::mat4, 6>> pointLightSpaceMatrices; // 6 view projection matrices per point light

    std::vector<unsigned int> spotLightShadowMaps; // one 2D depth map per spotlight
    std::vector<unsigned int> spotLightShadowFBOs;
    std::vector<glm::mat4> spotLightSpaceMatrices;
    glm::vec3 flashlightOffset = glm::vec3(-0.25f, -0.25f, 0.0f);
    
    //  Post processing states 
    bool isPostProcessing = true;
    unsigned int postProcessingFB;
    unsigned int texColorBuffer;
    unsigned int rboDepthStencil;
    unsigned int PPVBO;
    unsigned int PPVAO;
    unsigned int intermediateFBO;
    unsigned int intermediateTex;

    float exposure = 1.5f;

    bool enableSharpen = true;
    float sharpness = 0.1f;

    bool enableBlur = false;
    float blurStrength = 1.0f;

    bool enableEdgeDetection = false;
    float edgeDetectionStrength = 1.0f;

    bool enablePixelate = false;
    float pixelateResolution = 256.0f;

    // bloom states
    bool enableBloom = true;
    unsigned int bloomExtractFBO;
    unsigned int bloomExtractTex;
    unsigned int bloomBlurFBO[2];
    unsigned int bloomBlurTex[2];
    float bloomThreshold = 0.8f;
    float bloomBlurRadius = 3.0f;
    float bloomIntensity = 1.0f;

    // UI states 
    float scrWidth = 1600.0f;
    float scrHeight = 1200.0f;
    bool flashlightOn = false;
    int selectedObjectID = 0;
    int transformMode = 1; // ImGuizmo::WORLD
    int transformOperation = 7; // ImGuizmo::TRANSLATE
    bool flyCamLocked = false; // Tab toggles cursor lock
    bool rmbLooking = false; // hold right-click look in UI mode
    bool isWireframe = false;
    bool showLightSpheres = true;
    glm::vec3 clearColor = glm::vec3(0.1f, 0.15f, 0.2f);
    bool reloadShader = false;
    float cameraSpeed = 3.0f;
    bool vsync = true;
    float fov = 45.0f;
    glm::vec3 cameraPos;
    glm::vec3 cameraRot;
};

