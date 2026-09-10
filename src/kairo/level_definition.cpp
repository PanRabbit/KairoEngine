#include <kairo/engine_context.h>
#include <kairo/game_object.h>
#include <kairo/material.h>
#include <kairo/model.h>
#include <kairo/skybox.h>
#include <kairo/shadow_mapping.h>

#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>

void LoadLevelFromJson(EngineContext& engineContext, const std::string& path) 
{
    // clear existing 
    engineContext.sceneObjects.clear();
    engineContext.pointLightPositions.clear();
    engineContext.pointLightColors.clear();
    engineContext.pointLightIntensityMults.clear();
    engineContext.pointLightRadii.clear();
    engineContext.spotLightPositions.clear();
    engineContext.spotLightDirections.clear();
    engineContext.spotLightColors.clear();
    engineContext.spotLightIntensityMults.clear();
    engineContext.spotLightCutOffs.clear();
    engineContext.spotLightOuterCutOffs.clear();
    engineContext.spotLightRadii.clear();
    engineContext.flashlightIndex = -1;

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "ERROR: Could not open level file: " << path << "\n";
        return;
    }

    nlohmann::json j = nlohmann::json::parse(file);

    for (auto& [key, value] : j.items()) {
        if (key.find("GameObjects") != std::string::npos) {
            for (auto& [objKey, objValue] : value.items()) {
                auto* gameObject = new GameObject(objKey, engineContext.getModelByName(objValue["model"].get<std::string>()), engineContext.getMaterialByName(objValue["material"].get<std::string>())); 
                gameObject->position = glm::vec3(objValue["location"][0].get<float>(), objValue["location"][1].get<float>(), objValue["location"][2].get<float>()); 
                gameObject->setEulerXYZ(glm::vec3(glm::radians(objValue["rotation"][0].get<float>()), glm::radians(objValue["rotation"][1].get<float>()), glm::radians(objValue["rotation"][2].get<float>()))); 
                gameObject->scale = glm::vec3(objValue["scale"][0].get<float>(), objValue["scale"][1].get<float>(), objValue["scale"][2].get<float>()); 
                engineContext.sceneObjects[objKey] = std::move(std::unique_ptr<GameObject>(gameObject)); 
            }
        }
        if (key.find("SkyBox") != std::string::npos) {
            DefineSkyBox(engineContext, value["name"].get<std::string>());
        }
        if (key.find("PointLights") != std::string::npos) {
            for (auto& [lightKey, value] : value.items()) {
                engineContext.pointLightPositions.push_back(glm::vec3(value["position"][0].get<float>(), value["position"][1].get<float>(), value["position"][2].get<float>())); 
                engineContext.pointLightColors.push_back(glm::vec3(value["color"][0].get<float>(), value["color"][1].get<float>(), value["color"][2].get<float>())); 
                engineContext.pointLightIntensityMults.push_back(value["intensity"].get<float>());
                engineContext.pointLightRadii.push_back(value.value("radius", EngineContext::DEFAULT_POINT_LIGHT_RADIUS)); 
            }
        }
        if (key.find("SpotLights") != std::string::npos) {
            for (auto& [lightKey, value] : value.items()) {
                engineContext.spotLightPositions.push_back(glm::vec3(value["position"][0].get<float>(), value["position"][1].get<float>(), value["position"][2].get<float>()));
                engineContext.spotLightDirections.push_back(glm::vec3(value["direction"][0].get<float>(), value["direction"][1].get<float>(), value["direction"][2].get<float>()));
                engineContext.spotLightColors.push_back(glm::vec3(value["color"][0].get<float>(), value["color"][1].get<float>(), value["color"][2].get<float>()));
                engineContext.spotLightIntensityMults.push_back(value["intensity"].get<float>());
                engineContext.spotLightCutOffs.push_back(value.value("cutOff", EngineContext::DEFAULT_SPOT_CUT_OFF));
                engineContext.spotLightOuterCutOffs.push_back(value.value("outerCutOff", EngineContext::DEFAULT_SPOT_OUTER_CUT_OFF));
                engineContext.spotLightRadii.push_back(value.value("radius", EngineContext::DEFAULT_SPOT_LIGHT_RADIUS));
            }
        }
        if (key.find("SunDirection") != std::string::npos) {
            engineContext.sunDirection = glm::vec3(value[0].get<float>(), value[1].get<float>(), value[2].get<float>());
        }
        if (key.find("SunColor") != std::string::npos) {
            engineContext.clearColor = glm::vec3(value[0].get<float>(), value[1].get<float>(), value[2].get<float>());
        }
        if (key.find("TorchColor") != std::string::npos) {
            engineContext.torchColor = glm::vec3(value[0].get<float>(), value[1].get<float>(), value[2].get<float>());
        }
    }

    InitPointLightCubemaps(engineContext);
    InitSpotLightShadowMaps(engineContext);
}

void SaveLevelToJson(EngineContext& engineContext, const std::string& path){}