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


    for (int i = 0; i < 7; i++) {
        auto* cube = new GameObject("Cube_" + std::to_string(i),
                                    engineContext.models[1].get(),
                                    engineContext.materials[1].get());
        cube->position = engineContext.cubePositions[i];
        cube->rotation.y = glm::degrees(engineContext.cubeRotations[i]);
        cube->scale = glm::vec3(engineContext.cubeScales[i]);
        engineContext.sceneObjects.push_back(std::unique_ptr<GameObject>(cube));
    }

    auto* floorObj = new GameObject("Floor",
                                    engineContext.models[2].get(),
                                    engineContext.materials[2].get());
    floorObj->scale = glm::vec3(10.0f);
    engineContext.sceneObjects.push_back(std::unique_ptr<GameObject>(floorObj));

    auto* suzanneObj = new GameObject("Suzanne",
                                      engineContext.models[0].get(),
                                      engineContext.materials[0].get());
    suzanneObj->position = glm::vec3(0.0f, 2.0f, 0.0f);
    suzanneObj->rotation.y = glm::degrees(180.0f);
    suzanneObj->scale = glm::vec3(0.5f);
    engineContext.sceneObjects.push_back(std::unique_ptr<GameObject>(suzanneObj));
}