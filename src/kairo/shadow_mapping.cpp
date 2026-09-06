#include <kairo/shadow_mapping.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

void CreateSunDepthMapFBO(unsigned int& depthMapFBO, unsigned int& depthMapTexture) {
    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthMapTexture);

    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderSceneToDepthMap(EngineContext& engineContext) {
    float near_plane = 1.0f;
    float far_plane = 32.0f;

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    
    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    glm::mat4 lightView = glm::lookAt(engineContext.sunDirection * -10.0f, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // important to invert the direction of the light, otherwise the camera is pointing away from the scene
    engineContext.lightSpaceMatrix = lightProjection * lightView;

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, engineContext.shadowDepthMapFBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SHADOW FBO INCOMPLETE!\n";

    glClear(GL_DEPTH_BUFFER_BIT);


    Shader* shadowShader = engineContext.getShaderByName("dirShadowMapping");
    shadowShader->use();
    shadowShader->setMat4("lightSpaceMatrix", engineContext.lightSpaceMatrix);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.0f, 4.0f);

    // draw scene to shadow depth map
    for (auto& [name, obj] : engineContext.sceneObjects) {
        obj->drawShader(*engineContext.getShaderByName("dirShadowMapping"));
    }

    glDisable(GL_POLYGON_OFFSET_FILL);


    // prep for drawing scene to phong shader with shadow map
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, static_cast<GLuint>(engineContext.scrWidth), static_cast<GLuint>(engineContext.scrHeight));

    Shader* phongShader = engineContext.getShaderByName("phong");
    phongShader->use();
    phongShader->setMat4("lightSpaceMatrix", engineContext.lightSpaceMatrix);

    glActiveTexture(GL_TEXTURE0 + 99);
    glBindTexture(GL_TEXTURE_2D, engineContext.shadowDepthMapTexture);
    phongShader->setInt("shadowMap", 99); // set shadow map texture unit to 99 to avoid conflict with other textures
}

void InitPointLightCubemaps(EngineContext& engineContext) {
    
    unsigned int numPointLights = engineContext.pointLightPositions.size();
    engineContext.pointLightShadowCubemaps.resize(numPointLights);
    engineContext.pointLightShadowFBOs.resize(numPointLights);
    engineContext.pointLightSpaceMatrices.resize(numPointLights);

    for (unsigned int i = 0; i < numPointLights; ++i) {
        // Create depth cubemap
        glGenTextures(1, &engineContext.pointLightShadowCubemaps[i]);
        glBindTexture(GL_TEXTURE_CUBE_MAP, engineContext.pointLightShadowCubemaps[i]);
        
        for (unsigned int face = 0; face < 6; ++face) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_DEPTH_COMPONENT,
                        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        }
        
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        
        // Create FBO for this light
        glGenFramebuffers(1, &engineContext.pointLightShadowFBOs[i]);
        glBindFramebuffer(GL_FRAMEBUFFER, engineContext.pointLightShadowFBOs[i]);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                            engineContext.pointLightShadowCubemaps[i], 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Point light " << i << " shadow FBO incomplete!\n";
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
}

void RenderSceneToDepthCubemap(EngineContext& engineContext, unsigned int lightIndex) {
    // Perspective projection for cubemap faces: 90° FOV
    float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, 0.01f, engineContext.pointLightFarPlane);

    glm::vec3 lightPos = engineContext.pointLightPositions[lightIndex];  // USE PARAMETER
    std::array<glm::mat4, 6>& matrices = engineContext.pointLightSpaceMatrices[lightIndex];

    matrices[0] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f));
    matrices[1] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f));
    matrices[2] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f));
    matrices[3] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f));
    matrices[4] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f));
    matrices[5] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f));

    Shader* pointShadowShader = engineContext.getShaderByName("pointShadowMapping");
    pointShadowShader->use();
    
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);  // front-face culling for cubemaps
    
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    
    // NO LOOP - just render THIS light
    glBindFramebuffer(GL_FRAMEBUFFER, engineContext.pointLightShadowFBOs[lightIndex]);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    pointShadowShader->setVec3("lightPos", lightPos);
    pointShadowShader->setFloat("farPlane", engineContext.pointLightFarPlane);
    for (int face = 0; face < 6; ++face) {
        pointShadowShader->setMat4("lightSpaceMatrices[" + std::to_string(face) + "]", 
                                   matrices[face]);
    }
    
    for (auto& [name, obj] : engineContext.sceneObjects) {
        obj->drawShader(*pointShadowShader);
    }
    
    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, engineContext.scrWidth, engineContext.scrHeight);
}

