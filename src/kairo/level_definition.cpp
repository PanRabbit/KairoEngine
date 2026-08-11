// will eventually be a json file
void DefineLevel(EngineContext& engineContext) {
    std::vector<GameObject*>& sceneObjects = engineContext.sceneObjects;

    glm::vec3 cubePositions[] = {
        glm::vec3( 2.6f, 0.5f, 0.24f),
        glm::vec3( 1.7f, 1.0f, 1.96f),
        glm::vec3( 1.7f, 2.5f, 1.96f),
        glm::vec3( -0.67f, 0.5f, 2.0f),
        glm::vec3( -2.2f, 0.5f, 0.6f),
        glm::vec3( -2.8f, 0.5f, -0.23f),
        glm::vec3( -2.5f, 1.375f, 0.2f)
    };

    float cubeRotations[] = { -28.0f, 37.0f, 60.0f, -16.0f, -54.0f, -54.0f, -30.0f };

    float cubeScales[] = { 1.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.75f };

    // Point Lights
    glm::vec3 pointLightPositions[] = {
        glm::vec3( 2.0f, 1.5f, -0.85f),
        glm::vec3( -1.5f, 2.2f, 1.7f),
        glm::vec3( -1.8f, 0.5f, -1.5f),
    };
    glm::vec3 pointLightColors[] = {
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(0.19f, 0.75f, 1.0f),
        glm::vec3(1.0f, 0.33f, 0.43f),
    };
    float pointLightIntensityMults[] = { 8.0f, 16.0f, 4.0f };

    glm::vec3 sunDirection = glm::vec3(-0.2f, -1.0f, 0.5f);
    glm::vec3 lightColor;
    glm::vec3 torchColor = glm::vec3(1.0f, 0.95f, 0.8f);

    Model meshSuzanne("meshes/suzanne.obj");
    Model meshCube("meshes/prims/cube.obj");
    Model meshPlane("meshes/prims/plane.obj");
    Model meshSphere("meshes/prims/sphere.obj");

    for (int i = 0; i < 7; i++) {
        auto* cube = new GameObject("Cube_" + std::to_string(i), &meshCube, &woodMaterial);
        cube->position = cubePositions[i];
        cube->rotation.y = glm::degrees(cubeRotations[i]);
        cube->scale = glm::vec3(cubeScales[i]);
        sceneObjects.push_back(cube);
    }   

    auto* floorObj = new GameObject("Floor", &meshPlane, &floorMaterial);
    floorObj->scale = glm::vec3(10.0f);
    sceneObjects.push_back(floorObj);

    auto* suzanneObj = new GameObject("Suzanne", &meshSuzanne, &grungeMaterial);
    suzanneObj->position = glm::vec3(0.0f, 2.0f, 0.0f);
    suzanneObj->rotation.y = glm::degrees(180.0f);
    suzanneObj->scale = glm::vec3(0.5f);
    sceneObjects.push_back(suzanneObj);
}