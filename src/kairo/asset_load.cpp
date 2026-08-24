#include <kairo/engine_context.h>
#include <kairo/material.h>
#include <kairo/model.h>
#include <kairo/mesh.h>
#include <kairo/shader.h>
#include <kairo/texture.h>
#include <kairo/selection.h>
#include <kairo/input.h>
#include <kairo/UI.h>

// should eventually automate on every asset in dir

void AssetLoad(EngineContext& engineContext) {

    // ==========================================
    // LOAD SHADERS (insert into maps first so materials can reference them)
    // ==========================================
    engineContext.shaders["phong"] = std::make_unique<Shader>("shaders/vertex_shader.vs", "shaders/phong_shader.fs");
    engineContext.shaders["light"] = std::make_unique<Shader>("shaders/vertex_shader.vs", "shaders/light_shader.fs");
    engineContext.shaders["depth"] = std::make_unique<Shader>("shaders/vertex_shader.vs", "shaders/depth_visualiser.fs");
    engineContext.shaders["selection"] = std::make_unique<Shader>("shaders/selection.vs", "shaders/selection.fs");
    engineContext.shaders["singleColor"] = std::make_unique<Shader>("shaders/vertex_shader.vs", "shaders/single_color.fs");
    engineContext.shaders["postProcessing"] = std::make_unique<Shader>("shaders/post_processing.vs", "shaders/post_processing.fs");
    engineContext.shaders["skybox"] = std::make_unique<Shader>("shaders/skybox.vs", "shaders/skybox.fs");

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

    // ==========================================
    // MODELS (insert directly into maps)
    // ==========================================
    engineContext.models["suzanne"] = std::make_unique<Model>("meshes/suzanne.obj");
    engineContext.models["cube"] = std::make_unique<Model>("meshes/prims/cube.obj");
    engineContext.models["plane"] = std::make_unique<Model>("meshes/prims/plane.obj");
    engineContext.models["sphere"] = std::make_unique<Model>("meshes/prims/sphere.obj");
    engineContext.models["suzanneFlat"] = std::make_unique<Model>("meshes/suzanneFlat.obj");
}
