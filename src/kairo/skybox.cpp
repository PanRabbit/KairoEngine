#include <kairo/engine_context.h>
#include <kairo/texture.h>
#include <string>
#include <vector>

void DefineSkyBox(EngineContext& engineContext, std::string skyboxName)
{
    std::string skyboxPath = "textures/Cubemap/";
    std::vector<std::string> skyboxFaces = {
        skyboxPath + skyboxName + "/px.png",
        skyboxPath + skyboxName + "/nx.png",
        skyboxPath + skyboxName + "/py.png",
        skyboxPath + skyboxName + "/ny.png",
        skyboxPath + skyboxName + "/pz.png",
        skyboxPath + skyboxName + "/nz.png"
    };
    engineContext.skyboxTexture = std::make_unique<CubeMapTexture>(skyboxFaces);


    const float SKYBOX_SCALE = 100.0f;
    float skyboxVertices[] = {
        -SKYBOX_SCALE,  SKYBOX_SCALE, -SKYBOX_SCALE,  // front face
        -SKYBOX_SCALE, -SKYBOX_SCALE, -SKYBOX_SCALE,
         SKYBOX_SCALE, -SKYBOX_SCALE, -SKYBOX_SCALE,
         SKYBOX_SCALE, -SKYBOX_SCALE, -SKYBOX_SCALE,
         SKYBOX_SCALE,  SKYBOX_SCALE, -SKYBOX_SCALE,
        -SKYBOX_SCALE,  SKYBOX_SCALE, -SKYBOX_SCALE,

        -SKYBOX_SCALE, -SKYBOX_SCALE,  SKYBOX_SCALE,  // back face
        -SKYBOX_SCALE, -SKYBOX_SCALE, -SKYBOX_SCALE,
        -SKYBOX_SCALE,  SKYBOX_SCALE, -SKYBOX_SCALE,
        -SKYBOX_SCALE,  SKYBOX_SCALE, -SKYBOX_SCALE,
        -SKYBOX_SCALE,  SKYBOX_SCALE,  SKYBOX_SCALE,
        -SKYBOX_SCALE, -SKYBOX_SCALE,  SKYBOX_SCALE,

         SKYBOX_SCALE, -SKYBOX_SCALE, -SKYBOX_SCALE,  // right face
         SKYBOX_SCALE, -SKYBOX_SCALE,  SKYBOX_SCALE,
         SKYBOX_SCALE,  SKYBOX_SCALE,  SKYBOX_SCALE,
         SKYBOX_SCALE,  SKYBOX_SCALE,  SKYBOX_SCALE,
         SKYBOX_SCALE,  SKYBOX_SCALE, -SKYBOX_SCALE,
         SKYBOX_SCALE, -SKYBOX_SCALE, -SKYBOX_SCALE,

        -SKYBOX_SCALE, -SKYBOX_SCALE,  SKYBOX_SCALE,  // left face
        -SKYBOX_SCALE,  SKYBOX_SCALE,  SKYBOX_SCALE,
         SKYBOX_SCALE,  SKYBOX_SCALE,  SKYBOX_SCALE,
         SKYBOX_SCALE,  SKYBOX_SCALE,  SKYBOX_SCALE,
         SKYBOX_SCALE, -SKYBOX_SCALE,  SKYBOX_SCALE,
        -SKYBOX_SCALE, -SKYBOX_SCALE,  SKYBOX_SCALE,

        -SKYBOX_SCALE,  SKYBOX_SCALE, -SKYBOX_SCALE,  // top face
         SKYBOX_SCALE,  SKYBOX_SCALE, -SKYBOX_SCALE,
         SKYBOX_SCALE,  SKYBOX_SCALE,  SKYBOX_SCALE,
         SKYBOX_SCALE,  SKYBOX_SCALE,  SKYBOX_SCALE,
        -SKYBOX_SCALE,  SKYBOX_SCALE,  SKYBOX_SCALE,
        -SKYBOX_SCALE,  SKYBOX_SCALE, -SKYBOX_SCALE,

        -SKYBOX_SCALE, -SKYBOX_SCALE, -SKYBOX_SCALE,  // bottom face
        -SKYBOX_SCALE, -SKYBOX_SCALE,  SKYBOX_SCALE,
         SKYBOX_SCALE, -SKYBOX_SCALE, -SKYBOX_SCALE,
         SKYBOX_SCALE, -SKYBOX_SCALE, -SKYBOX_SCALE,
        -SKYBOX_SCALE, -SKYBOX_SCALE,  SKYBOX_SCALE,
         SKYBOX_SCALE, -SKYBOX_SCALE,  SKYBOX_SCALE
    };

    engineContext.skyboxVBO = 0;
    glGenBuffers(1, &engineContext.skyboxVBO);
    glBindBuffer(GL_ARRAY_BUFFER, engineContext.skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    engineContext.skyboxVAO = 0;
    glGenVertexArrays(1, &engineContext.skyboxVAO);
    glBindVertexArray(engineContext.skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, engineContext.skyboxVBO);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}