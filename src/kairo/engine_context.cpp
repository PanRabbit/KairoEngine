#include "kairo/engine_context.h"
#include <stdexcept>

Shader* EngineContext::getShaderByName(const std::string& name) {
    auto it = shaders.find(name);
    if (it == shaders.end()) {
        throw std::runtime_error("Shader not found: " + name);
    }
    return it->second.get();
}

Material* EngineContext::getMaterialByName(const std::string& name) {
    auto it = materials.find(name);
    if (it == materials.end()) {
        throw std::runtime_error("Material not found: " + name);
    }
    return it->second.get();
}

Model* EngineContext::getModelByName(const std::string& name) {
    auto it = models.find(name);
    if (it == models.end()) {
        throw std::runtime_error("Model not found: " + name);
    }
    return it->second.get();
}

GameObject* EngineContext::getGameObjectByName(const std::string& name) {
    auto it = sceneObjects.find(name);
    if (it == sceneObjects.end()) {
        throw std::runtime_error("Game object not found: " + name);
    }
    return it->second.get();
}
