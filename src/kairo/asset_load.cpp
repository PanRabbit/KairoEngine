// should eventually automate on every asset in dir

void AssetLoad(EngineContext& engineContext) {

    auto phongShader = std::make_unique<Shader>("shaders/vertex_shader.vs", "shaders/phong_shader.fs");

    auto lightShader = std::make_unique<Shader>("shaders/vertex_shader.vs", "shaders/light_shader.fs");

    auto depthShader = std::make_unique<Shader>("shaders/vertex_shader.vs", "shaders/depth_visualiser.fs");

    auto selectionShader = std::make_unique<Shader>("shaders/selection.vs", "shaders/selection.fs");


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



    engineContext.shaders.push_back(std::move(phongShader));
    engineContext.shaders.push_back(std::move(lightShader));
    engineContext.shaders.push_back(std::move(depthShader));
    engineContext.shaders.push_back(std::move(selectionShader));



    // Load models (indices: 0=suzanne, 1=cube, 2=plane, 3=sphere)
    auto meshSuzanne = std::make_unique<Model>("meshes/suzanne.obj");
    engineContext.models.push_back(std::move(meshSuzanne));

    auto meshCube = std::make_unique<Model>("meshes/prims/cube.obj");
    engineContext.models.push_back(std::move(meshCube));

    auto meshPlane = std::make_unique<Model>("meshes/prims/plane.obj");
    engineContext.models.push_back(std::move(meshPlane));

    auto meshSphere = std::make_unique<Model>("meshes/prims/sphere.obj");
    engineContext.models.push_back(std::move(meshSphere));
}