#include <kairo/engine_context.h>
#include <kairo/material.h>
#include <kairo/model.h>
#include <kairo/mesh.h>
#include <kairo/shader.h>
#include <kairo/texture.h>
#include <kairo/selection.h>
#include <kairo/input.h>
#include <kairo/UI.h>
#include <filesystem>

static bool IsModelFile(const std::filesystem::path& path)
{
    static const std::string kValidExtensions[] = { ".obj"}; // valid model extensions
    std::string pathExtension = path.extension().string(); // get the extension of the provided path
    for (const std::string& extension : kValidExtensions) {
        if (pathExtension == extension) {
            return true;
        }
    }
    return false;
}

// IMPORTANT!!! This should eventually automate on every asset in dir
void AssetLoad(EngineContext& engineContext) {

    // ==========================================
    // LOAD SHADERS (insert into maps first so materials can reference them)
    // ==========================================
    engineContext.shaders["phong"] = std::make_unique<Shader>("shaders/vertex_shader.vs", "shaders/phong_shader.fs");
    engineContext.shaders["light"] = std::make_unique<Shader>("shaders/vertex_shader.vs", "shaders/light_shader.fs");
    engineContext.shaders["selection"] = std::make_unique<Shader>("shaders/selection.vs", "shaders/selection.fs");
    engineContext.shaders["postProcessing"] = std::make_unique<Shader>("shaders/post_processing.vs", "shaders/post_processing.fs");
    engineContext.shaders["skybox"] = std::make_unique<Shader>("shaders/skybox.vs", "shaders/skybox.fs");
    engineContext.shaders["dirShadowMapping"] = std::make_unique<Shader>("shaders/dir_shadow_mapping.vs", "shaders/dir_shadow_mapping.fs");
    engineContext.shaders["pointShadowMapping"] = std::make_unique<Shader>("shaders/point_shadow_mapping.vs", "shaders/point_shadow_mapping.fs", "shaders/point_shadow_mapping.gs");
    engineContext.shaders["bloomExtract"] = std::make_unique<Shader>("shaders/post_processing.vs", "shaders/bloom_extract.fs");
    engineContext.shaders["bloomBlur"] = std::make_unique<Shader>("shaders/post_processing.vs", "shaders/bloom_blur.fs");
    // ==========================================
    // MATERIALS (reference shaders by name)
    // ==========================================
    auto lightMaterial = std::make_unique<Material>(engineContext.getShaderByName("light"));
    lightMaterial->loadFromJson("materials/light.json");
    engineContext.materials["light"] = std::move(lightMaterial);

    std::error_code ec;
    const std::filesystem::path materialsRoot("materials");
    for (const auto& entry : std::filesystem::directory_iterator(materialsRoot, ec)) {
        // check if file is a json file
        if (entry.path().extension() != ".json")
            continue;
        // check if the material is already loaded
        const std::string key = entry.path().stem().string();
        if (engineContext.materials.count(key))
            continue;
        auto material = std::make_unique<Material>(engineContext.getShaderByName("phong"));
        material->loadFromJson(entry.path().generic_string());
        engineContext.materials[key] = std::move(material);
    }
    // ==========================================
    // MODEL CATALOG (paths only; meshes load on demand)
    // ==========================================

    const std::filesystem::path meshesRoot("meshes");
    for (const auto& entry : std::filesystem::recursive_directory_iterator(meshesRoot, ec)) {
        if (!entry.is_regular_file() || !IsModelFile(entry.path()))
            continue;
        const std::string path = entry.path().generic_string();
        const auto rel = std::filesystem::relative(entry.path(), meshesRoot);
        const std::string folder = rel.parent_path().generic_string();
        if (engineContext.availableModels.count(path))
            continue;
        engineContext.availableModels[path] = folder;
    }

    engineContext.LoadModel(EngineContext::GIZMO_SPHERE_PATH);
}
