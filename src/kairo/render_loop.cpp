#include "kairo/camera.h"
#include "kairo/engine_context.h"
#include "kairo/shader.h"
#include "kairo/UI.h"
#include "kairo/input.h"
#include "kairo/shadow_mapping.h"

float lastFrame = 0.0f;
glm::vec3 lightColor;

void RenderLoop(GLFWwindow* window, EngineContext& engineContext) {
            // Process time & Input
            float currentFrame = static_cast<float>(glfwGetTime());
            engineContext.deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;
    
            // Calculate Matrices for this frame
            engineContext.view = engineContext.camera.GetViewMatrix();
            engineContext.centerView = engineContext.camera.GetCenterViewMatrix();
            engineContext.projection = glm::perspective(glm::radians(engineContext.camera.Zoom), engineContext.scrWidth / engineContext.scrHeight, 0.1f, 500.0f);
    
            // Process UI toggles 
            if (engineContext.isWireframe) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            } else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }

            // ==========================================
            // SET CONTEXT AND UNIFORMS
            // ==========================================
            Shader& phongShader = *engineContext.getShaderByName("phong");
            Shader& lightShader = *engineContext.getShaderByName("light");
            Shader& postProcessingShader = *engineContext.getShaderByName("postProcessing");

            phongShader.use();
    
            // Directional light (sun)
            glm::vec3 sunColor = glm::vec3(engineContext.clearColor[0], engineContext.clearColor[1], engineContext.clearColor[2]);
            phongShader.setVec3("dirLight.direction", engineContext.sunDirection);
            phongShader.setFloat("dirLight.intensity", 5.0f);
            phongShader.setVec3("dirLight.ambient", sunColor * 0.15f);
            phongShader.setVec3("dirLight.diffuse", sunColor * 1.0f);
            phongShader.setVec3("dirLight.specular", sunColor * 1.0f);

            // Skybox
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, engineContext.skyboxTexture->ID);
            phongShader.setInt("skybox", 0);
       
    
            // Point lights
            int pointLightCount = static_cast<int>(engineContext.pointLightPositions.size());
            if (pointLightCount > EngineContext::MAX_POINT_LIGHTS) pointLightCount = EngineContext::MAX_POINT_LIGHTS;
            phongShader.setInt("numPointLights", pointLightCount);
            for(int i = 0; i < pointLightCount; i++)
            {
                lightColor = engineContext.pointLightColors[i];
                std::string uniformID = "pointLights[" + std::to_string(i) + "].";
                phongShader.setVec3(uniformID + "ambient", lightColor * 0.15f);
                phongShader.setVec3(uniformID + "diffuse", lightColor * 1.0f);
                phongShader.setVec3(uniformID + "specular", lightColor * 1.0f);
                phongShader.setVec3(uniformID + "position", engineContext.pointLightPositions[i]);
                phongShader.setFloat(uniformID + "radius", engineContext.pointLightRadii[i]);
                phongShader.setFloat(uniformID + "intensity", engineContext.pointLightIntensityMults[i]);

                int shadowMapUnit = EngineContext::POINT_SHADOW_TEXTURE_UNIT + i;
                phongShader.setInt("pointShadowMaps[" + std::to_string(i) + "]", shadowMapUnit);
                glActiveTexture(GL_TEXTURE0 + shadowMapUnit);
                glBindTexture(GL_TEXTURE_CUBE_MAP, engineContext.pointLightShadowCubemaps[i]);
            }

            // Slot 0 is always the flashlight; intensity 0 when off so world spots keep their indices.
            int spotLightCount = static_cast<int>(engineContext.spotLightPositions.size());
            if (spotLightCount > EngineContext::MAX_SPOT_LIGHTS) spotLightCount = EngineContext::MAX_SPOT_LIGHTS;
            if (spotLightCount > 0) {
                engineContext.spotLightPositions[0] = engineContext.camera.Position + engineContext.flashlightOffset;
                engineContext.spotLightDirections[0] = engineContext.camera.Front;
                engineContext.spotLightColors[0] = engineContext.torchColor;
                engineContext.spotLightIntensityMults[0] = engineContext.flashlightOn ? EngineContext::FLASHLIGHT_INTENSITY : 0.0f;
                engineContext.spotLightCutOffs[0] = EngineContext::FLASHLIGHT_CUT_OFF;
                engineContext.spotLightOuterCutOffs[0] = EngineContext::FLASHLIGHT_OUTER_CUT_OFF;
                engineContext.spotLightRadii[0] = EngineContext::FLASHLIGHT_RADIUS;
            }

            phongShader.setInt("numSpotLights", spotLightCount);
            for (int i = 0; i < spotLightCount; i++)
            {
                lightColor = engineContext.spotLightColors[i];
                std::string uniformID = "spotLights[" + std::to_string(i) + "].";
                phongShader.setVec3(uniformID + "ambient", lightColor * 0.15f);
                phongShader.setVec3(uniformID + "diffuse", lightColor * 1.0f);
                phongShader.setVec3(uniformID + "specular", lightColor * 1.0f);
                phongShader.setVec3(uniformID + "position", engineContext.spotLightPositions[i]);
                phongShader.setVec3(uniformID + "direction", engineContext.spotLightDirections[i]);
                phongShader.setFloat(uniformID + "cutOff", glm::cos(glm::radians(engineContext.spotLightCutOffs[i])));
                phongShader.setFloat(uniformID + "outerCutOff", glm::cos(glm::radians(engineContext.spotLightOuterCutOffs[i])));
                phongShader.setFloat(uniformID + "radius", engineContext.spotLightRadii[i]);
                phongShader.setFloat(uniformID + "intensity", engineContext.spotLightIntensityMults[i]);

                int shadowMapUnit = EngineContext::SPOT_SHADOW_TEXTURE_UNIT + i;
                phongShader.setInt("spotShadowMaps[" + std::to_string(i) + "]", shadowMapUnit);
                glActiveTexture(GL_TEXTURE0 + shadowMapUnit);
                glBindTexture(GL_TEXTURE_2D, engineContext.spotLightShadowMaps[i]);
            }
    
            // update camera matrices
            phongShader.setMat4("view", engineContext.view);
            phongShader.setMat4("projection", engineContext.projection);
            phongShader.setVec3("viewPos", engineContext.camera.Position);
            phongShader.setFloat("scrWidth", engineContext.scrWidth);
            phongShader.setFloat("scrHeight", engineContext.scrHeight);
    
            // ==========================================
            // BEGIN DRAW
            // ==========================================
            processInput(window, engineContext);

            // render scene to depth map for shadow mapping
            RenderSceneToDepthMap(engineContext);

            for(int i = 0; i < pointLightCount; i++)
            { 
                RenderSceneToDepthCubemap(engineContext, i);
            }

            for (int i = 0; i < spotLightCount; i++)
            {
                if (i == 0 && !engineContext.flashlightOn)
                    continue;
                RenderSceneToSpotDepthMap(engineContext, i);
            }

            // draw scene to post-processing framebuffer (draw scene to a texture)
            if (engineContext.isPostProcessing) {
                glBindFramebuffer(GL_FRAMEBUFFER, engineContext.postProcessingFB);
            }

            glClearColor(sunColor.r, sunColor.g, sunColor.b, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);

            // draw skybox
            engineContext.getShaderByName("skybox")->use();
            engineContext.getShaderByName("skybox")->setMat4("projection", engineContext.projection);
            engineContext.getShaderByName("skybox")->setMat4("view", engineContext.centerView);
            glBindVertexArray(engineContext.skyboxVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, engineContext.skyboxTexture->ID);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glDepthMask(GL_TRUE);
    
            // Render all game objects
            for (auto& [name, obj] : engineContext.sceneObjects) {
                    obj->draw(phongShader, engineContext.selectedObjectID);
            }
    
            // Render point lights
            lightShader.use();
            lightShader.setMat4("view", engineContext.view);
            lightShader.setMat4("projection", engineContext.projection);

            if (engineContext.showLightSpheres) {
                for(unsigned int i = 0; i < engineContext.pointLightPositions.size(); i++)
                {   
                    lightColor = engineContext.pointLightColors[i];
                    lightShader.setVec3("Color", lightColor);
                    glm::mat4 lightModel = glm::mat4(1.0f); 
                    lightModel = glm::translate(lightModel, engineContext.pointLightPositions[i]); 
                    lightModel = glm::scale(lightModel, glm::vec3(0.2f)); 
                    lightShader.setMat4("model", lightModel);
                    engineContext.getModelByName("sphere")->draw(*engineContext.getMaterialByName("light"));
                }

                for (int i = 1; i < spotLightCount; i++)
                {
                    lightColor = engineContext.spotLightColors[i];
                    lightShader.setVec3("Color", lightColor);
                    glm::mat4 lightModel = glm::mat4(1.0f);
                    lightModel = glm::translate(lightModel, engineContext.spotLightPositions[i]);
                    lightModel = glm::scale(lightModel, glm::vec3(0.2f));
                    lightShader.setMat4("model", lightModel);
                    engineContext.getModelByName("sphere")->draw(*engineContext.getMaterialByName("light"));
                }

                {
                    lightShader.setVec3("Color", sunColor);
                    glm::mat4 sunModel = glm::translate(glm::mat4(1.0f), engineContext.sunHandlePosition());
                    sunModel = glm::scale(sunModel, glm::vec3(0.35f));
                    lightShader.setMat4("model", sunModel);
                    engineContext.getModelByName("sphere")->draw(*engineContext.getMaterialByName("light"));
                }
            }

            // draw post-processing quad and render processed texture to screen
            if (engineContext.isPostProcessing) {
                // resolve MSAA FBO to intermediate regular texture via blit
                glBindFramebuffer(GL_READ_FRAMEBUFFER, engineContext.postProcessingFB);
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, engineContext.intermediateFBO);
                glBlitFramebuffer(0, 0, static_cast<int>(engineContext.scrWidth), static_cast<int>(engineContext.scrHeight),
                                  0, 0, static_cast<int>(engineContext.scrWidth), static_cast<int>(engineContext.scrHeight),
                                  GL_COLOR_BUFFER_BIT, GL_NEAREST);

                unsigned int bloomResult = engineContext.intermediateTex;
                glDisable(GL_DEPTH_TEST);
                glBindVertexArray(engineContext.PPVAO);

                if (engineContext.enableBloom) {
                    // bloom extract
                    Shader& bloomExtractShader = *engineContext.getShaderByName("bloomExtract");

                    glBindFramebuffer(GL_FRAMEBUFFER, engineContext.bloomExtractFBO);
                    bloomExtractShader.use();
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, engineContext.intermediateTex);
                    bloomExtractShader.setInt("screenTexture", 0);
                    bloomExtractShader.setFloat("threshold", engineContext.bloomThreshold);
                    glDrawArrays(GL_TRIANGLES, 0, 6);

                    // bloom blur
                    Shader& bloomBlurShader = *engineContext.getShaderByName("bloomBlur");
                    bloomBlurShader.use();
                    bloomBlurShader.setInt("image", 0);

                    bool horizontal = true;
                    bool firstIteration = true;
                    for (int i = 0; i < 10; i++) // 10 passes = 5 horizontal + 5 vertical (the size of the weight array)
                    {
                        glBindFramebuffer(GL_FRAMEBUFFER, engineContext.bloomBlurFBO[horizontal]);
                        bloomBlurShader.setBool("horizontal", horizontal);
                        glBindTexture(GL_TEXTURE_2D, firstIteration ? engineContext.bloomExtractTex : engineContext.bloomBlurTex[!horizontal]); // first iteration uses extract texture, subsequent iterations use the previous buffer
                        bloomBlurShader.setFloat("radius", engineContext.bloomBlurRadius);
                        glDrawArrays(GL_TRIANGLES, 0, 6);
                        horizontal = !horizontal;
                        firstIteration = false;
                    }
                    bloomResult = engineContext.bloomBlurTex[!horizontal]; // last buffer (ends on vertical blur) becomes the final result
                }

                // bind default framebuffer and draw post-processing quad
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                postProcessingShader.use();
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, engineContext.intermediateTex);
                postProcessingShader.setInt("screenTexture", 0);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, bloomResult);
                postProcessingShader.setInt("bloomBlur", 1);
                postProcessingShader.setFloat("time", currentFrame);
                postProcessingShader.setFloat("scrWidth", engineContext.scrWidth);
                postProcessingShader.setFloat("scrHeight", engineContext.scrHeight);
                postProcessingShader.setFloat("exposure", engineContext.exposure);
                postProcessingShader.setBool("enableSharpen", engineContext.enableSharpen);
                postProcessingShader.setFloat("sharpness", engineContext.sharpness);
                postProcessingShader.setBool("enableBlur", engineContext.enableBlur);
                postProcessingShader.setFloat("blurStrength", engineContext.blurStrength);
                postProcessingShader.setBool("enableEdgeDetection", engineContext.enableEdgeDetection);
                postProcessingShader.setFloat("edgeDetectionStrength", engineContext.edgeDetectionStrength);
                postProcessingShader.setBool("enablePixelate", engineContext.enablePixelate);
                postProcessingShader.setFloat("pixelateResolution", engineContext.pixelateResolution);
                postProcessingShader.setBool("enableBloom", engineContext.enableBloom);
                postProcessingShader.setFloat("bloomIntensity", engineContext.bloomIntensity);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                glBindVertexArray(0);    
            }

            // ==========================================
            // UPDATE UI STATES
            // ==========================================
            engineContext.cameraSpeed = engineContext.camera.MovementSpeed;
            engineContext.fov = engineContext.camera.Zoom;
            engineContext.cameraPos = engineContext.camera.Position;
            engineContext.cameraRot = glm::vec3(engineContext.camera.Pitch, engineContext.camera.Yaw, 0.0f);

            // ==========================================
            // END FRAME UI & SWAP
            // ==========================================
            glfwSwapInterval(engineContext.vsync ? 1 : 0);
            RenderUI(engineContext);
    
            glfwPollEvents();
            glfwSwapBuffers(window);
}