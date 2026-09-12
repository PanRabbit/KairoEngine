#include <kairo/engine_context.h>
#include <kairo/game_object.h>
#include <kairo/material.h>
#include <kairo/model.h>
#include <kairo/skybox.h>
#include <kairo/shadow_mapping.h>
#include <kairo/level_definition.h>

#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cctype>

// empty private namespace for level definition functions
namespace {

    // get the keys of a map and sort them (keeps objects in a consistent order)
    template<typename Map>
    std::vector<std::string> SortedMapKeys(const Map& map) {
        std::vector<std::string> names;
        names.reserve(map.size());
        for (const auto& [name, _] : map)
            names.push_back(name);
        std::sort(names.begin(), names.end());
        return names;
    }

    // make a unique object name by appending a number to the base name if it already exists
    std::string MakeUniqueObjectName(const EngineContext& engineContext, const std::string& base) {
        if (engineContext.sceneObjects.find(base) == engineContext.sceneObjects.end())
            return base;
        int i = 0;
        while (engineContext.sceneObjects.find(base + "_" + std::to_string(i)) != engineContext.sceneObjects.end())
            ++i;
        return base + "_" + std::to_string(i);
    }

    // spawn an object in front of the camera
    glm::vec3 SpawnInFrontOfCamera(const EngineContext& engineContext) {
        return engineContext.camera.Position + engineContext.camera.Front * 4.0f;
    }

    std::string DefaultMaterialName(const EngineContext& engineContext) {
        if (engineContext.materials.count("wood"))
            return "wood";
        for (const auto& [name, _] : engineContext.materials) {
            if (name != "light")
                return name;
        }
        return {};
    }

    std::vector<std::string> ReadObjectMaterialNames(const nlohmann::json& objValue) {
        if (objValue.contains("materials") && objValue["materials"].is_array())
            return objValue["materials"].get<std::vector<std::string>>();
        if (objValue.contains("material") && objValue["material"].is_string())
            return { objValue["material"].get<std::string>() };
        return {};
    }

    void LoadPostProcess(EngineContext& engineContext, const nlohmann::json& j) {
        if (!j.contains("PostProcess") || !j["PostProcess"].is_object())
            return;
        const auto& p = j["PostProcess"];
        engineContext.isPostProcessing = p.value("enabled", true);
        engineContext.exposure = p.value("exposure", 1.5f);
        engineContext.enableBloom = p.value("enableBloom", true);
        engineContext.bloomThreshold = p.value("bloomThreshold", 0.8f);
        engineContext.bloomBlurRadius = p.value("bloomBlurRadius", 3.0f);
        engineContext.bloomIntensity = p.value("bloomIntensity", 1.0f);
        engineContext.enableSharpen = p.value("enableSharpen", true);
        engineContext.sharpness = p.value("sharpness", 0.1f);
        engineContext.enableBlur = p.value("enableBlur", false);
        engineContext.blurStrength = p.value("blurStrength", 1.0f);
        engineContext.enableEdgeDetection = p.value("enableEdgeDetection", false);
        engineContext.edgeDetectionStrength = p.value("edgeDetectionStrength", 1.0f);
        engineContext.enablePixelate = p.value("enablePixelate", false);
        engineContext.pixelateResolution = p.value("pixelateResolution", 256.0f);
    }

    nlohmann::json SavePostProcess(const EngineContext& engineContext) {
        return {
            { "enabled", engineContext.isPostProcessing },
            { "exposure", engineContext.exposure },
            { "enableBloom", engineContext.enableBloom },
            { "bloomThreshold", engineContext.bloomThreshold },
            { "bloomBlurRadius", engineContext.bloomBlurRadius },
            { "bloomIntensity", engineContext.bloomIntensity },
            { "enableSharpen", engineContext.enableSharpen },
            { "sharpness", engineContext.sharpness },
            { "enableBlur", engineContext.enableBlur },
            { "blurStrength", engineContext.blurStrength },
            { "enableEdgeDetection", engineContext.enableEdgeDetection },
            { "edgeDetectionStrength", engineContext.edgeDetectionStrength },
            { "enablePixelate", engineContext.enablePixelate },
            { "pixelateResolution", engineContext.pixelateResolution }
        };
    }

    std::string SanitizeLevelStem(std::string name) {
        auto slash = name.find_last_of("/\\");
        if (slash != std::string::npos)
            name = name.substr(slash + 1);
        constexpr const char* ext = ".json";
        if (name.size() >= 5 && name.compare(name.size() - 5, 5, ext) == 0)
            name = name.substr(0, name.size() - 5);
        std::string out;
        out.reserve(name.size());
        for (char c : name) {
            if (std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-')
                out.push_back(c);
            else if (c == ' ')
                out.push_back('_');
        }
        return out;
    }

}

void LoadLevelFromJson(EngineContext& engineContext, const std::string& path) 
{
    
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "ERROR: Could not open level file: " << path << "\n";
        return;
    }

    nlohmann::json j;
    try {
        j = nlohmann::json::parse(file);
    } catch (const std::exception& e) {
        std::cout << "ERROR: Failed to parse level file " << path << ": " << e.what() << "\n";
        return;
    }

    // clear existing level data
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
    engineContext.selectedObjectID = 0;

    // Set params for flashlight (Slot 0 is always the flashlight; world spots are appended after it.)
    engineContext.spotLightPositions.emplace_back(0.0f);
    engineContext.spotLightDirections.emplace_back(0.0f, 0.0f, -1.0f);
    engineContext.spotLightColors.emplace_back(1.0f);
    engineContext.spotLightIntensityMults.push_back(0.0f);
    engineContext.spotLightCutOffs.push_back(EngineContext::FLASHLIGHT_CUT_OFF);
    engineContext.spotLightOuterCutOffs.push_back(EngineContext::FLASHLIGHT_OUTER_CUT_OFF);
    engineContext.spotLightRadii.push_back(EngineContext::FLASHLIGHT_RADIUS);

    for (auto& [key, value] : j.items()) {
        if (key.find("GameObjects") != std::string::npos) {
            for (auto& [objKey, objValue] : value.items()) {
                const std::string modelName = objValue["model"].get<std::string>();
                auto* gameObject = new GameObject(
                    objKey,
                    engineContext.getModelByName(modelName),
                    {},
                    modelName,
                    ReadObjectMaterialNames(objValue)
                );
                SyncObjectMaterialSlots(engineContext, *gameObject);
                gameObject->position = glm::vec3(objValue["location"][0].get<float>(), objValue["location"][1].get<float>(), objValue["location"][2].get<float>());
                gameObject->setEulerXYZ(glm::radians(glm::vec3(objValue["rotation"][0].get<float>(), objValue["rotation"][1].get<float>(), objValue["rotation"][2].get<float>())));
                gameObject->scale = glm::vec3(objValue["scale"][0].get<float>(), objValue["scale"][1].get<float>(), objValue["scale"][2].get<float>());
                engineContext.sceneObjects[objKey] = std::unique_ptr<GameObject>(gameObject);
            }
        }
        if (key.find("SkyBox") != std::string::npos) {
            DefineSkyBox(engineContext, value["name"].get<std::string>());
        }
        if (key.find("PointLights") != std::string::npos) {
            for (auto& [lightKey, lightValue] : value.items()) {
                engineContext.pointLightPositions.push_back(glm::vec3(lightValue["position"][0].get<float>(), lightValue["position"][1].get<float>(), lightValue["position"][2].get<float>()));
                engineContext.pointLightColors.push_back(glm::vec3(lightValue["color"][0].get<float>(), lightValue["color"][1].get<float>(), lightValue["color"][2].get<float>()));
                engineContext.pointLightIntensityMults.push_back(lightValue["intensity"].get<float>());
                engineContext.pointLightRadii.push_back(lightValue.value("radius", EngineContext::DEFAULT_POINT_LIGHT_RADIUS));
            }
        }
        if (key.find("SpotLights") != std::string::npos) {
            for (auto& [lightKey, lightValue] : value.items()) {
                engineContext.spotLightPositions.push_back(glm::vec3(lightValue["position"][0].get<float>(), lightValue["position"][1].get<float>(), lightValue["position"][2].get<float>()));
                engineContext.spotLightDirections.push_back(glm::vec3(lightValue["direction"][0].get<float>(), lightValue["direction"][1].get<float>(), lightValue["direction"][2].get<float>()));
                engineContext.spotLightColors.push_back(glm::vec3(lightValue["color"][0].get<float>(), lightValue["color"][1].get<float>(), lightValue["color"][2].get<float>()));
                engineContext.spotLightIntensityMults.push_back(lightValue["intensity"].get<float>());
                engineContext.spotLightCutOffs.push_back(lightValue.value("cutOff", EngineContext::DEFAULT_SPOT_CUT_OFF));
                engineContext.spotLightOuterCutOffs.push_back(lightValue.value("outerCutOff", EngineContext::DEFAULT_SPOT_OUTER_CUT_OFF));
                engineContext.spotLightRadii.push_back(lightValue.value("radius", EngineContext::DEFAULT_SPOT_LIGHT_RADIUS));
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

    LoadPostProcess(engineContext, j);
    engineContext.currentLevelPath = path;

    InitPointLightCubemaps(engineContext);
    InitSpotLightShadowMaps(engineContext);
}

void SaveLevelToJson(EngineContext& engineContext, const std::string& path)
{
    nlohmann::json j;

    // Save skybox name (if any)
    if (!engineContext.currentSkyboxName.empty())
        j["SkyBox"] = { { "name", engineContext.currentSkyboxName } };

    j["SunDirection"] = { engineContext.sunDirection.x, engineContext.sunDirection.y, engineContext.sunDirection.z };
    j["SunColor"] = { engineContext.clearColor.x, engineContext.clearColor.y, engineContext.clearColor.z };
    j["TorchColor"] = { engineContext.torchColor.x, engineContext.torchColor.y, engineContext.torchColor.z };
    j["PostProcess"] = SavePostProcess(engineContext);

    // Save game objects
    nlohmann::json objects = nlohmann::json::object();
    std::vector<std::string> objectNames = SortedMapKeys(engineContext.sceneObjects);
    for (const std::string& objKey : objectNames) {
        GameObject* obj = engineContext.sceneObjects[objKey].get();
        const glm::vec3 rotationDeg = glm::degrees(obj->getEulerXYZ());
        objects[objKey] = {
            { "model", obj->modelName },
            { "materials", obj->materialNames },
            { "location", { obj->position.x, obj->position.y, obj->position.z } },
            { "rotation", { rotationDeg.x, rotationDeg.y, rotationDeg.z } },
            { "scale", { obj->scale.x, obj->scale.y, obj->scale.z } }
        };
    }
    j["GameObjects"] = objects;

    // Save point lights
    nlohmann::json pointLights = nlohmann::json::object();
    for (size_t i = 0; i < engineContext.pointLightPositions.size(); ++i) {
        pointLights["light" + std::to_string(i)] = {
            { "position", { engineContext.pointLightPositions[i].x, engineContext.pointLightPositions[i].y, engineContext.pointLightPositions[i].z } },
            { "color", { engineContext.pointLightColors[i].x, engineContext.pointLightColors[i].y, engineContext.pointLightColors[i].z } },
            { "intensity", engineContext.pointLightIntensityMults[i] },
            { "radius", engineContext.pointLightRadii[i] }
        };
    }
    j["PointLights"] = pointLights;

    // Save spot lights
    nlohmann::json spotLights = nlohmann::json::object();
    for (size_t i = 1; i < engineContext.spotLightPositions.size(); ++i) {
        spotLights["spot" + std::to_string(i - 1)] = {
            { "position", { engineContext.spotLightPositions[i].x, engineContext.spotLightPositions[i].y, engineContext.spotLightPositions[i].z } },
            { "direction", { engineContext.spotLightDirections[i].x, engineContext.spotLightDirections[i].y, engineContext.spotLightDirections[i].z } },
            { "color", { engineContext.spotLightColors[i].x, engineContext.spotLightColors[i].y, engineContext.spotLightColors[i].z } },
            { "intensity", engineContext.spotLightIntensityMults[i] },
            { "cutOff", engineContext.spotLightCutOffs[i] },
            { "outerCutOff", engineContext.spotLightOuterCutOffs[i] },
            { "radius", engineContext.spotLightRadii[i] }
        };
    }
    j["SpotLights"] = spotLights;

    // Create directories if needed
    std::filesystem::path outPath(path);
    if (outPath.has_parent_path())
        std::filesystem::create_directories(outPath.parent_path());

    std::ofstream file(path);
    if (!file.is_open()) {
        std::cout << "ERROR: Could not write level file: " << path << "\n";
        return;
    }
    file << j.dump(4) << "\n";
    engineContext.currentLevelPath = path;
    std::cout << "Saved level: " << path << "\n";
}

// Save level as a new file
bool SaveLevelAs(EngineContext& engineContext, const std::string& rawName)
{
    const std::string path = MakeLevelPath(rawName);
    if (path.empty()) {
        std::cout << "ERROR: Invalid level name.\n";
        return false;
    }
    SaveLevelToJson(engineContext, path);
    return true;
}

// Make a level path from just a name
std::string MakeLevelPath(const std::string& rawName)
{
    const std::string stem = SanitizeLevelStem(rawName);
    if (stem.empty())
        return {};
    return "levels/" + stem + ".json";
}

// list all level files in the levels directory
std::vector<std::string> ListLevelFiles()
{
    std::vector<std::string> files;
    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator("levels", ec)) {
        if (!entry.is_regular_file())
            continue;
        if (entry.path().extension() == ".json")
            files.push_back(entry.path().generic_string());
    }
    std::sort(files.begin(), files.end());
    return files;
}

// list all skybox names in the textures sets/Cubemap directory
std::vector<std::string> ListSkyboxNames()
{
    std::vector<std::string> names;
    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator("textures/Cubemap", ec)) {
        if (!entry.is_directory())
            continue;
        if (std::filesystem::exists(entry.path() / "px.png"))
            names.push_back(entry.path().filename().string());
    }
    std::sort(names.begin(), names.end());
    return names;
}

// list all model names grouped by folder
std::map<std::string, std::vector<std::string>> ListModelsByFolder(const EngineContext& engineContext)
{
    std::map<std::string, std::vector<std::string>> grouped;
    for (const auto& [name, _] : engineContext.models) {
        auto it = engineContext.modelFolders.find(name);
        const std::string folder = (it != engineContext.modelFolders.end() && !it->second.empty())
            ? it->second
            : "(root)";
        grouped[folder].push_back(name);
    }
    for (auto& [_, names] : grouped)
        std::sort(names.begin(), names.end());
    return grouped;
}

std::vector<std::string> ListMaterialNames(const EngineContext& engineContext)
{
    std::vector<std::string> names = SortedMapKeys(engineContext.materials);
    names.erase(std::remove(names.begin(), names.end(), "light"), names.end());
    return names;
}

void SyncObjectMaterialSlots(EngineContext& engineContext, GameObject& object)
{
    const std::string fillName = DefaultMaterialName(engineContext);
    if (fillName.empty()) {
        std::cout << "ERROR: No material available to assign to object.\n";
        return;
    }

    Material* fill = engineContext.getMaterialByName(fillName);
    const size_t slots = object.model ? object.model->materialSlotCount() : 1;
    object.syncMaterialSlots(slots, fill, fillName);
    for (size_t i = 0; i < object.materialNames.size(); ++i) {
        auto it = engineContext.materials.find(object.materialNames[i]);
        if (it == engineContext.materials.end())
            object.setMaterial(i, fill, fillName);
        else
            object.setMaterial(i, it->second.get(), object.materialNames[i]);
    }
}

void AddObjectToLevel(EngineContext& engineContext, const std::string& modelName)
{
    Model* model = engineContext.getModelByName(modelName);
    if (DefaultMaterialName(engineContext).empty()) {
        std::cout << "ERROR: No material available to assign to new object.\n";
        return;
    }

    const std::string objectName = MakeUniqueObjectName(engineContext, modelName);
    auto object = std::make_unique<GameObject>(objectName, model, std::vector<Material*>{}, modelName);
    SyncObjectMaterialSlots(engineContext, *object);
    object->position = SpawnInFrontOfCamera(engineContext);
    engineContext.selectedObjectID = object->id;
    engineContext.sceneObjects[objectName] = std::move(object);
}

void AddPointLightToLevel(EngineContext& engineContext)
{
    if (static_cast<int>(engineContext.pointLightPositions.size()) >= EngineContext::MAX_POINT_LIGHTS) {
        std::cout << "ERROR: Point light limit reached.\n";
        return;
    }
    engineContext.pointLightPositions.push_back(SpawnInFrontOfCamera(engineContext));
    engineContext.pointLightColors.push_back(glm::vec3(1.0f));
    engineContext.pointLightIntensityMults.push_back(4.0f);
    engineContext.pointLightRadii.push_back(EngineContext::DEFAULT_POINT_LIGHT_RADIUS);
    InitPointLightCubemaps(engineContext);
    engineContext.selectedObjectID = EngineContext::PointLightSelectID(
        static_cast<int>(engineContext.pointLightPositions.size()) - 1
    );
}

void AddSpotLightToLevel(EngineContext& engineContext)
{
    if (static_cast<int>(engineContext.spotLightPositions.size()) >= EngineContext::MAX_SPOT_LIGHTS) {
        std::cout << "ERROR: Spot light limit reached.\n";
        return;
    }
    engineContext.spotLightPositions.push_back(SpawnInFrontOfCamera(engineContext));
    engineContext.spotLightDirections.push_back(engineContext.camera.Front);
    engineContext.spotLightColors.push_back(glm::vec3(1.0f, 0.85f, 0.55f));
    engineContext.spotLightIntensityMults.push_back(16.0f);
    engineContext.spotLightCutOffs.push_back(EngineContext::DEFAULT_SPOT_CUT_OFF);
    engineContext.spotLightOuterCutOffs.push_back(EngineContext::DEFAULT_SPOT_OUTER_CUT_OFF);
    engineContext.spotLightRadii.push_back(EngineContext::DEFAULT_SPOT_LIGHT_RADIUS);
    InitSpotLightShadowMaps(engineContext);
    engineContext.selectedObjectID = EngineContext::SpotLightSelectID(
        static_cast<int>(engineContext.spotLightPositions.size()) - 1
    );
}

void DuplicateSelectedObject(EngineContext& engineContext)
{
    const int id = engineContext.selectedObjectID;
    if (id == 0 || id == EngineContext::SUN_SELECT_ID)
        return;

    if (EngineContext::IsPointLightSelectID(id)) {
        const int index = EngineContext::PointLightIndexFromSelectID(id);
        if (index < 0 || index >= static_cast<int>(engineContext.pointLightPositions.size()))
            return;
        if (static_cast<int>(engineContext.pointLightPositions.size()) >= EngineContext::MAX_POINT_LIGHTS) {
            std::cout << "ERROR: Point light limit reached.\n";
            return;
        }
        engineContext.pointLightPositions.push_back(engineContext.pointLightPositions[index]);
        engineContext.pointLightColors.push_back(engineContext.pointLightColors[index]);
        engineContext.pointLightIntensityMults.push_back(engineContext.pointLightIntensityMults[index]);
        engineContext.pointLightRadii.push_back(engineContext.pointLightRadii[index]);
        InitPointLightCubemaps(engineContext);
        engineContext.selectedObjectID = EngineContext::PointLightSelectID(
            static_cast<int>(engineContext.pointLightPositions.size()) - 1
        );
        return;
    }

    if (EngineContext::IsSpotLightSelectID(id)) {
        const int index = EngineContext::SpotLightIndexFromSelectID(id);
        if (index < 1 || index >= static_cast<int>(engineContext.spotLightPositions.size()))
            return;
        if (static_cast<int>(engineContext.spotLightPositions.size()) >= EngineContext::MAX_SPOT_LIGHTS) {
            std::cout << "ERROR: Spot light limit reached.\n";
            return;
        }
        engineContext.spotLightPositions.push_back(engineContext.spotLightPositions[index]);
        engineContext.spotLightDirections.push_back(engineContext.spotLightDirections[index]);
        engineContext.spotLightColors.push_back(engineContext.spotLightColors[index]);
        engineContext.spotLightIntensityMults.push_back(engineContext.spotLightIntensityMults[index]);
        engineContext.spotLightCutOffs.push_back(engineContext.spotLightCutOffs[index]);
        engineContext.spotLightOuterCutOffs.push_back(engineContext.spotLightOuterCutOffs[index]);
        engineContext.spotLightRadii.push_back(engineContext.spotLightRadii[index]);
        InitSpotLightShadowMaps(engineContext);
        engineContext.selectedObjectID = EngineContext::SpotLightSelectID(
            static_cast<int>(engineContext.spotLightPositions.size()) - 1
        );
        return;
    }

    GameObject* source = engineContext.getGameObjectByID(id);
    if (!source)
        return;

    const std::string objectName = MakeUniqueObjectName(engineContext, source->name);
    auto clone = std::make_unique<GameObject>(
        objectName,
        source->model,
        source->materials,
        source->modelName,
        source->materialNames
    );
    SyncObjectMaterialSlots(engineContext, *clone);
    clone->setFromMatrix(source->getTransformMatrix());
    engineContext.selectedObjectID = clone->id;
    engineContext.sceneObjects[objectName] = std::move(clone);
}

void DeleteSelectedFromLevel(EngineContext& engineContext)
{
    const int id = engineContext.selectedObjectID;
    if (id == 0 || id == EngineContext::SUN_SELECT_ID)
        return;

    if (EngineContext::IsPointLightSelectID(id)) {
        const int index = EngineContext::PointLightIndexFromSelectID(id);
        if (index < 0 || index >= static_cast<int>(engineContext.pointLightPositions.size()))
            return;
        engineContext.pointLightPositions.erase(engineContext.pointLightPositions.begin() + index);
        engineContext.pointLightColors.erase(engineContext.pointLightColors.begin() + index);
        engineContext.pointLightIntensityMults.erase(engineContext.pointLightIntensityMults.begin() + index);
        engineContext.pointLightRadii.erase(engineContext.pointLightRadii.begin() + index);
        InitPointLightCubemaps(engineContext);
        engineContext.selectedObjectID = 0;
        return;
    }

    if (EngineContext::IsSpotLightSelectID(id)) {
        const int index = EngineContext::SpotLightIndexFromSelectID(id);
        if (index < 1 || index >= static_cast<int>(engineContext.spotLightPositions.size()))
            return;
        engineContext.spotLightPositions.erase(engineContext.spotLightPositions.begin() + index);
        engineContext.spotLightDirections.erase(engineContext.spotLightDirections.begin() + index);
        engineContext.spotLightColors.erase(engineContext.spotLightColors.begin() + index);
        engineContext.spotLightIntensityMults.erase(engineContext.spotLightIntensityMults.begin() + index);
        engineContext.spotLightCutOffs.erase(engineContext.spotLightCutOffs.begin() + index);
        engineContext.spotLightOuterCutOffs.erase(engineContext.spotLightOuterCutOffs.begin() + index);
        engineContext.spotLightRadii.erase(engineContext.spotLightRadii.begin() + index);
        InitSpotLightShadowMaps(engineContext);
        engineContext.selectedObjectID = 0;
        return;
    }

    if (GameObject* obj = engineContext.getGameObjectByID(id)) {
        engineContext.sceneObjects.erase(obj->name);
        engineContext.selectedObjectID = 0;
    }
}
