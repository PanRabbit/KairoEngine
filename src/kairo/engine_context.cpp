#include "kairo/engine_context.h"
#include <filesystem>
#include <iostream>
#include <stdexcept>

Shader* EngineContext::getShaderByName(const std::string& name) {
    auto assetPointer = shaders.find(name);
    if (assetPointer == shaders.end()) {
        throw std::runtime_error("Shader not found: " + name);
    }
    return assetPointer->second.get();
}

Material* EngineContext::getMaterialByName(const std::string& name) {
    auto assetPointer = materials.find(name);
    if (assetPointer == materials.end()) {
        throw std::runtime_error("Material not found: " + name);
    }
    return assetPointer->second.get();
}

Model* EngineContext::LoadModel(const std::string& path) {
    if (path.empty()) {
        std::cout << "ERROR: Model path is empty" << std::endl;
        return nullptr;
    }

    auto loaded = models.find(path);
    if (loaded != models.end())
        return loaded->second.get();

    std::error_code ec;
    if (!std::filesystem::exists(path, ec)) {
        std::cout << "ERROR: Model not found: " << path << std::endl;
        return nullptr;
    }

    auto model = std::make_unique<Model>(path);
    Model* pointer = model.get();
    models[path] = std::move(model);
    return pointer;
}

void EngineContext::unloadUnusedModels(const std::unordered_set<std::string>& keepPaths) {
    for (auto it = models.begin(); it != models.end(); ) {
        if (keepPaths.count(it->first) || it->first == GIZMO_SPHERE_PATH) {
            ++it;
            continue;
        }
        it = models.erase(it);
    }
}

GameObject* EngineContext::getGameObjectByID(int id) {
    for (auto& [name, object] : sceneObjects) {
        if (object->id == id) {
            return object.get();
        }
    }
    std::cout << "Game object not found: ID of " << id << std::endl;
    return nullptr;
}
