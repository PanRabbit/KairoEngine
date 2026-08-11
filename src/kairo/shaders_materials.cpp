void ShadersAndMaterials(EngineContext& engineContext) {
    // ==========================================
    // SHADER COMPILATION & LINKING
    // ==========================================
    Shader phongShader("shaders/vertex_shader.vs", "shaders/phong_shader.fs");
    Shader lightShader("shaders/vertex_shader.vs", "shaders/light_shader.fs");
    Shader depthShader("shaders/vertex_shader.vs", "shaders/depth_visualiser.fs");
    Shader selectionShader("shaders/selection.vs", "shaders/selection.fs");

    std::vector<Shader*> allShaders = {&lightShader, &phongShader, &selectionShader};

    // ==========================================
    // MATERIALS
    // ==========================================
    Material woodMaterial(&phongShader);
    woodMaterial.loadFromJson("materials/container.json");

    Material floorMaterial(&phongShader);
    floorMaterial.loadFromJson("materials/floor.json");

    Material mikuMaterial(&phongShader);
    mikuMaterial.loadFromJson("materials/mikuCube.json");

    Material grungeMaterial(&phongShader);
    grungeMaterial.loadFromJson("materials/grunge.json");

    Material lightMaterial(&lightShader);
    lightMaterial.loadFromJson("materials/light.json");

    std::vector<Material*> allMaterials = {&woodMaterial, &floorMaterial, &mikuMaterial, &grungeMaterial, &lightMaterial};
}