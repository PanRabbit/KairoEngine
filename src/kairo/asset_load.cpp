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
    // LOAD SHADERS (But don't push to context yet)
    // ==========================================
    auto phongShader = std::make_unique<Shader>("shaders/vertex_shader.vs", "shaders/phong_shader.fs");
    auto lightShader = std::make_unique<Shader>("shaders/vertex_shader.vs", "shaders/light_shader.fs");
    auto depthShader = std::make_unique<Shader>("shaders/vertex_shader.vs", "shaders/depth_visualiser.fs");
    auto selectionShader = std::make_unique<Shader>("shaders/selection.vs", "shaders/selection.fs");
    auto singleColorShader = std::make_unique<Shader>("shaders/vertex_shader.vs", "shaders/single_color.fs");
    auto postProcessingShader = std::make_unique<Shader>("shaders/post_processing.vs", "shaders/post_processing.fs");
    auto skyboxShader = std::make_unique<Shader>("shaders/skybox.vs", "shaders/skybox.fs");
    // ==========================================
    // MATERIALS
    // ==========================================
    auto woodMaterial = std::make_unique<Material>(phongShader.get());
    woodMaterial->loadFromJson("materials/container.json");
    engineContext.materials.push_back(std::move(woodMaterial));

    auto floorMaterial = std::make_unique<Material>(phongShader.get());
    floorMaterial->loadFromJson("materials/floor.json");
    engineContext.materials.push_back(std::move(floorMaterial));

    auto mikuMaterial = std::make_unique<Material>(phongShader.get());
    mikuMaterial->loadFromJson("materials/mikuCube.json");
    engineContext.materials.push_back(std::move(mikuMaterial));

    auto grungeMaterial = std::make_unique<Material>(phongShader.get());
    grungeMaterial->loadFromJson("materials/grunge.json");
    engineContext.materials.push_back(std::move(grungeMaterial));

    auto lightMaterial = std::make_unique<Material>(lightShader.get());
    lightMaterial->loadFromJson("materials/light.json");
    engineContext.materials.push_back(std::move(lightMaterial));

    auto grassMaterial = std::make_unique<Material>(phongShader.get());
    grassMaterial->loadFromJson("materials/grass.json");
    engineContext.materials.push_back(std::move(grassMaterial));

    // ==========================================
    // PUSH SHADERS TO CONTEXT (After materials loaded)
    // ==========================================
    engineContext.shaders.push_back(std::move(phongShader));
    engineContext.shaders.push_back(std::move(lightShader));
    engineContext.shaders.push_back(std::move(depthShader));
    engineContext.shaders.push_back(std::move(selectionShader));
    engineContext.shaders.push_back(std::move(singleColorShader));
    engineContext.shaders.push_back(std::move(postProcessingShader));
    engineContext.shaders.push_back(std::move(skyboxShader));
    // ==========================================
    // MODELS
    // ==========================================
    auto meshSuzanne = std::make_unique<Model>("meshes/suzanne.obj");
    engineContext.models.push_back(std::move(meshSuzanne));

    auto meshCube = std::make_unique<Model>("meshes/prims/cube.obj");
    engineContext.models.push_back(std::move(meshCube));

    auto meshPlane = std::make_unique<Model>("meshes/prims/plane.obj");
    engineContext.models.push_back(std::move(meshPlane));

    auto meshSphere = std::make_unique<Model>("meshes/prims/sphere.obj");
    engineContext.models.push_back(std::move(meshSphere));

    auto meshSuzanneFlat = std::make_unique<Model>("meshes/suzanneFlat.obj");
    engineContext.models.push_back(std::move(meshSuzanneFlat));
}