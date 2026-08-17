// will eventually be a json file
void DefineLevel(EngineContext& engineContext) {

    engineContext.cubePositions.assign({
        glm::vec3( 2.6f, 0.5f, 0.24f),
        glm::vec3( 1.7f, 1.0f, 1.96f),
        glm::vec3( 1.7f, 2.5f, 1.96f),
        glm::vec3( -0.67f, 0.5f, 2.0f),
        glm::vec3( -2.2f, 0.5f, 0.6f),
        glm::vec3( -2.8f, 0.5f, -0.23f),
        glm::vec3( -2.5f, 1.375f, 0.2f)
    });

    engineContext.cubeRotations.assign({ -28.0f, 37.0f, 60.0f, -16.0f, -54.0f, -54.0f, -30.0f });

    engineContext.cubeScales.assign({ 1.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.75f });

    engineContext.grassPositions.assign({
        glm::vec3(3.7f, 0.2f, -1.2f),
    });

    // Point Lights
    engineContext.pointLightPositions.assign({
        glm::vec3( 2.0f, 1.5f, -0.85f),
        glm::vec3( -1.5f, 2.2f, 1.7f),
        glm::vec3( -1.8f, 0.5f, -1.5f),
    });
    engineContext.pointLightColors.assign({
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(0.19f, 0.75f, 1.0f),
        glm::vec3(1.0f, 0.33f, 0.43f),
    });
    engineContext.pointLightIntensityMults.assign({ 8.0f, 16.0f, 4.0f });

    engineContext.sunDirection = glm::vec3(-0.2f, -1.0f, 0.5f);
    engineContext.torchColor = glm::vec3(1.0f, 0.95f, 0.8f);

    // Cubes
    for (int i = 0; i < engineContext.cubePositions.size(); i++) {
        auto* cube = new GameObject("Cube_" + std::to_string(i), engineContext.models[1].get(), engineContext.materials[0].get());
        cube->position = engineContext.cubePositions[i];
        cube->rotation.y = glm::radians(engineContext.cubeRotations[i]);
        cube->scale = glm::vec3(engineContext.cubeScales[i]);
        engineContext.sceneObjects.push_back(std::unique_ptr<GameObject>(cube));
    }

    // Floor
    auto* floorObj = new GameObject("Floor", engineContext.models[2].get(), engineContext.materials[1].get());
    floorObj->scale = glm::vec3(10.0f);
    engineContext.sceneObjects.push_back(std::unique_ptr<GameObject>(floorObj));

    // Grass
    for (int i = 0; i < engineContext.grassPositions.size(); i++) {
        for (int j = 0; j < 10; j++) {
            auto* grassObj = new GameObject("Grass_" + std::to_string(i), engineContext.models[2].get(), engineContext.materials[5].get());
            grassObj->scale = glm::vec3(float(rand() % 100) / 100.0f + 0.5f);
            grassObj->position = engineContext.grassPositions[i];
            grassObj->rotation.x = glm::radians(90.0f);
            grassObj->rotation.z = glm::radians(float(rand() % 360));
            engineContext.sceneObjects.push_back(std::unique_ptr<GameObject>(grassObj));
        }
    }

    // Suzanne
    auto* suzanneObj = new GameObject("Suzanne", engineContext.models[0].get(), engineContext.materials[3].get());
    suzanneObj->position = glm::vec3(0.0f, 2.0f, 0.0f);
    suzanneObj->rotation.y = glm::radians(180.0f);
    suzanneObj->scale = glm::vec3(0.5f);
    engineContext.sceneObjects.push_back(std::unique_ptr<GameObject>(suzanneObj));
}