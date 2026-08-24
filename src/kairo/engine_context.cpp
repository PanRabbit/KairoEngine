#include "kairo/engine_context.h"
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

Model* EngineContext::getModelByName(const std::string& name) {
    auto assetPointer = models.find(name);
    if (assetPointer == models.end()) {
        throw std::runtime_error("Model not found: " + name);
    }
    return assetPointer->second.get();
}

GameObject* EngineContext::getGameObjectByName(const std::string& name) {
    auto assetPointer = sceneObjects.find(name);
    if (assetPointer == sceneObjects.end()) {
        throw std::runtime_error("Game object not found: " + name);
    }
    return assetPointer->second.get();
}

GameObject* EngineContext::getGameObjectByID(int id) {
    for (auto& [name, object] : sceneObjects) {
        if (object->id == id) {
            return object.get();
        }
    }
    throw std::runtime_error("Game object not found: ID of " + std::to_string(id));
}