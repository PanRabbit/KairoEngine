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
#include <iostream>

static bool IsModelFile(const std::string& path)
{
    static const std::string kValidExtensions[] = { ".obj", ".fbx", ".gltf", ".glb" }; // valid model extensions
    std::string pathExtension = std::filesystem::path(path).extension().string(); // get the extension of the provided path
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
    auto woodMaterial = std::make_unique<Material>(engineContext.getShaderByName("phong"));
    woodMaterial->loadFromJson("materials/container.json");
    engineContext.materials["wood"] = std::move(woodMaterial);

    auto floorMaterial = std::make_unique<Material>(engineContext.getShaderByName("phong"));
    floorMaterial->loadFromJson("materials/floor.json");
    engineContext.materials["floor"] = std::move(floorMaterial);

    auto mikuMaterial = std::make_unique<Material>(engineContext.getShaderByName("phong"));
    mikuMaterial->loadFromJson("materials/mikuCube.json");
    engineContext.materials["miku"] = std::move(mikuMaterial);

    auto grungeMaterial = std::make_unique<Material>(engineContext.getShaderByName("phong"));
    grungeMaterial->loadFromJson("materials/grunge.json");
    engineContext.materials["grunge"] = std::move(grungeMaterial);

    auto lightMaterial = std::make_unique<Material>(engineContext.getShaderByName("light"));
    lightMaterial->loadFromJson("materials/light.json");
    engineContext.materials["light"] = std::move(lightMaterial);

    auto grassMaterial = std::make_unique<Material>(engineContext.getShaderByName("phong"));
    grassMaterial->loadFromJson("materials/grass.json");
    engineContext.materials["grass"] = std::move(grassMaterial);

    auto brickMaterial = std::make_unique<Material>(engineContext.getShaderByName("phong"));
    brickMaterial->loadFromJson("materials/brick.json");
    engineContext.materials["brick"] = std::move(brickMaterial);

    // ==========================================
    // MODELS (insert directly into maps)
    // ==========================================

    const std::filesystem::path meshesRoot("meshes");
    std::error_code ec;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(meshesRoot, ec)) {
        if (!entry.is_regular_file() || !IsModelFile(entry.path()))
            continue;
        const auto rel = std::filesystem::relative(entry.path(), meshesRoot);
        const std::string key = rel.stem().string();          // "cube"
        const std::string folder = rel.parent_path().generic_string(); // "" or "prims"
        if (engineContext.models.count(key)) {
            std::cerr << "Duplicate model name '" << key
                      << "' skipped: " << entry.path() << '\n';
            continue;
        }
        engineContext.models[key] = std::make_unique<Model>(entry.path().generic_string());
        engineContext.modelFolders[key] = folder;
    }


}
