#include <kairo/shadow_mapping.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

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

    glActiveTexture(GL_TEXTURE0 + EngineContext::SUN_SHADOW_TEXTURE_UNIT);
    glBindTexture(GL_TEXTURE_2D, engineContext.shadowDepthMapTexture);
    phongShader->setInt("shadowMap", EngineContext::SUN_SHADOW_TEXTURE_UNIT);
}

void InitPointLightCubemaps(EngineContext& engineContext) {
    // clear existing point light cubemaps and FBOs
    for (unsigned int tex : engineContext.pointLightShadowCubemaps) {
        if (tex != 0) glDeleteTextures(1, &tex);
    }
    for (unsigned int fbo : engineContext.pointLightShadowFBOs) {
        if (fbo != 0) glDeleteFramebuffers(1, &fbo);
    }
    engineContext.pointLightShadowCubemaps.clear();
    engineContext.pointLightShadowFBOs.clear();
    engineContext.pointLightSpaceMatrices.clear();

    unsigned int numPointLights = engineContext.pointLightPositions.size();
    if (numPointLights > EngineContext::MAX_POINT_LIGHTS)
        numPointLights = EngineContext::MAX_POINT_LIGHTS;
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
    float farPlane = glm::max(engineContext.pointLightRadii[lightIndex], 0.02f);
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, 0.01f, farPlane);

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
    pointShadowShader->setFloat("farPlane", farPlane);
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

void InitSpotLightShadowMaps(EngineContext& engineContext)
{
    for (unsigned int fbo : engineContext.spotLightShadowFBOs) {
        if (fbo != 0) glDeleteFramebuffers(1, &fbo);
    }
    for (unsigned int tex : engineContext.spotLightShadowMaps) {
        if (tex != 0) glDeleteTextures(1, &tex);
    }

    unsigned int numSpotLights = engineContext.spotLightPositions.size();
    if (numSpotLights > EngineContext::MAX_SPOT_LIGHTS)
        numSpotLights = EngineContext::MAX_SPOT_LIGHTS;

    engineContext.spotLightShadowMaps.assign(numSpotLights, 0);
    engineContext.spotLightShadowFBOs.assign(numSpotLights, 0);
    engineContext.spotLightSpaceMatrices.resize(numSpotLights);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    for (unsigned int i = 0; i < numSpotLights; ++i) {
        glGenFramebuffers(1, &engineContext.spotLightShadowFBOs[i]);
        glGenTextures(1, &engineContext.spotLightShadowMaps[i]);

        glBindTexture(GL_TEXTURE_2D, engineContext.spotLightShadowMaps[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        glBindFramebuffer(GL_FRAMEBUFFER, engineContext.spotLightShadowFBOs[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, engineContext.spotLightShadowMaps[i], 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Spot light " << i << " shadow FBO incomplete!\n";
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void RenderSceneToSpotDepthMap(EngineContext& engineContext, unsigned int lightIndex)
{
    glm::vec3 position = engineContext.spotLightPositions[lightIndex];
    glm::vec3 direction = engineContext.spotLightDirections[lightIndex];
    float fov = glm::clamp(engineContext.spotLightOuterCutOffs[lightIndex] * 2.0f, 1.0f, 179.0f);
    float nearPlane = 0.1f;
    float farPlane = glm::max(engineContext.spotLightRadii[lightIndex], nearPlane + 0.01f);
    glm::mat4 projection = glm::perspective(glm::radians(fov), 1.0f, nearPlane, farPlane);

    glm::vec3 dir = (glm::dot(direction, direction) < 1e-8f)
        ? glm::vec3(0.0f, -1.0f, 0.0f)
        : glm::normalize(direction);
    glm::vec3 up = (glm::abs(dir.y) > 0.99f) ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
    engineContext.spotLightSpaceMatrices[lightIndex] = projection * glm::lookAt(position, position + dir, up);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, engineContext.spotLightShadowFBOs[lightIndex]);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SPOT SHADOW FBO INCOMPLETE!\n";

    glClear(GL_DEPTH_BUFFER_BIT);

    Shader* shadowShader = engineContext.getShaderByName("dirShadowMapping");
    shadowShader->use();
    shadowShader->setMat4("lightSpaceMatrix", engineContext.spotLightSpaceMatrices[lightIndex]);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.0f, 4.0f);

    for (auto& [name, obj] : engineContext.sceneObjects) {
        obj->drawShader(*shadowShader);
    }

    glDisable(GL_POLYGON_OFFSET_FILL);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, static_cast<GLuint>(engineContext.scrWidth), static_cast<GLuint>(engineContext.scrHeight));

    Shader* phongShader = engineContext.getShaderByName("phong");
    phongShader->use();
    phongShader->setMat4("spotLightSpaceMatrices[" + std::to_string(lightIndex) + "]",
                         engineContext.spotLightSpaceMatrices[lightIndex]);
}
