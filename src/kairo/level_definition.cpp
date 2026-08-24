#include <kairo/engine_context.h>
#include <kairo/game_object.h>
#include <kairo/material.h>
#include <kairo/model.h>
#include "src/kairo/skybox.cpp"

#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>

void LoadLevelFromJson(EngineContext& engineContext, const std::string& path) 
{
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
                gameObject->rotation = glm::vec3(glm::radians(objValue["rotation"][0].get<float>()), glm::radians(objValue["rotation"][1].get<float>()), glm::radians(objValue["rotation"][2].get<float>())); 
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
}

void SaveLevelToJson(EngineContext& engineContext, const std::string& path){}