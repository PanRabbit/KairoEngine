#include "kairo/engine_context.h"
#include "kairo/shader.h"
#include "kairo/UI.h"
#include "kairo/input.h"

float lastFrame = 0.0f;
glm::vec3 lightColor;

void RenderLoop(GLFWwindow* window, EngineContext& engineContext) {
            // Process time & Input
            float currentFrame = static_cast<float>(glfwGetTime());
            engineContext.deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;
    
            // Calculate Matrices for this frame
            engineContext.view = engineContext.camera.GetViewMatrix();
            engineContext.projection = glm::perspective(glm::radians(engineContext.camera.Zoom), engineContext.scrWidth / engineContext.scrHeight, 0.1f, 100.0f);
    
            // Process UI toggles 
            if (engineContext.isWireframe) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            } else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
    
            // Post processing conditions (add more conditions here)
            if (engineContext.isWireframe) {
                engineContext.isPostProcessing = false;
            } else {
                engineContext.isPostProcessing = true;
            }
            // ==========================================
            // LIGHTS AND CAMERA VARIABLES
            // ==========================================
            Shader& phongShader = *engineContext.shaders[0];
            Shader& lightShader = *engineContext.shaders[1];
            Shader& singleColorShader = *engineContext.shaders[4];
            Material& lightMaterial = *engineContext.materials[4];


            phongShader.use();
    
            // Directional light (sun)
            glm::vec3 sunColor = glm::vec3(engineContext.clearColor[0], engineContext.clearColor[1], engineContext.clearColor[2]);
            phongShader.setVec3("dirLight.direction", engineContext.sunDirection);
            phongShader.setFloat("dirLight.intensity", 5.0f);
            phongShader.setVec3("dirLight.ambient", sunColor * 0.15f);
            phongShader.setVec3("dirLight.diffuse", sunColor * 1.0f);
            phongShader.setVec3("dirLight.specular", sunColor * 1.0f);
    
            // Point lights
            for(unsigned int i = 0; i < 3; i++)
            {
                lightColor = engineContext.pointLightColors[i];
                std::string uniformID = "pointLights[" + std::to_string(i) + "].";
                phongShader.setVec3(uniformID + "ambient", lightColor * 0.15f);
                phongShader.setVec3(uniformID + "diffuse", lightColor * 1.0f);
                phongShader.setVec3(uniformID + "specular", lightColor * 1.0f);
                phongShader.setVec3(uniformID + "position", engineContext.pointLightPositions[i]);
                phongShader.setFloat(uniformID + "radius", 8.0f);
                phongShader.setFloat(uniformID + "intensity", 0.5f * engineContext.pointLightIntensityMults[i]);
            }
    
            // Flashlight
            if (engineContext.flashlightOn)
            {
                phongShader.setVec3("spotLight.position", engineContext.camera.Position);
                phongShader.setVec3("spotLight.direction", engineContext.camera.Front);
                phongShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
                phongShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(20.0f)));
                phongShader.setVec3("spotLight.ambient", engineContext.torchColor * 0.15f);
                phongShader.setVec3("spotLight.diffuse", engineContext.torchColor * 1.0f);
                phongShader.setVec3("spotLight.specular", engineContext.torchColor * 1.0f);
                phongShader.setFloat("spotLight.radius", 64.0f);
                phongShader.setFloat("spotLight.intensity", 32.0f);
            }
            else
            {
                phongShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(0.0f)));
                phongShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(0.0f)));
                phongShader.setFloat("spotLight.intensity", 0.0f);
            }
    
            // update camera matrices
            phongShader.setMat4("view", engineContext.view);
            phongShader.setMat4("projection", engineContext.projection);
            phongShader.setVec3("viewPos", engineContext.camera.Position);
    
            // ==========================================
            // BEGIN DRAW
            // ==========================================
            processInput(window, engineContext);

            // draw scene to post-processing framebuffer (draw scene to a texture)
            if (engineContext.isPostProcessing) {
                glBindFramebuffer(GL_FRAMEBUFFER, engineContext.postProcessingFB);
            }
            glClearColor(sunColor.r, sunColor.g, sunColor.b, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
    
            // Render all regular objects
            for (auto& obj : engineContext.sceneObjects) {
                    obj->draw(phongShader, engineContext.selectedObjectID);
            }
    
            // Render light spheres
            lightShader.use();
            lightShader.setMat4("view", engineContext.view);
            lightShader.setMat4("projection", engineContext.projection);
    
            for(unsigned int i = 0; i < 3; i++)
            {   
                lightColor = engineContext.pointLightColors[i];
                lightShader.setVec3("Color", lightColor);
                glm::mat4 lightModel = glm::mat4(1.0f); 
                lightModel = glm::translate(lightModel, engineContext.pointLightPositions[i]); 
                lightModel = glm::scale(lightModel, glm::vec3(0.2f)); 
                lightShader.setMat4("model", lightModel);
                engineContext.models[3]->draw(*engineContext.materials[4]);
            }

            if (engineContext.isPostProcessing) {
            // unbind post-processing framebuffer and bind default framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // draw post-processing quad
            engineContext.shaders[5]->use();
            glBindVertexArray(engineContext.PPVAO); // bind post-processing VAO
            glDisable(GL_DEPTH_TEST);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, engineContext.texColorBuffer);
            engineContext.shaders[5]->setInt("screenTexture", 0);
            engineContext.shaders[5]->setFloat("time", currentFrame);
            engineContext.shaders[5]->setFloat("scrWidth", engineContext.scrWidth);
            engineContext.shaders[5]->setFloat("scrHeight", engineContext.scrHeight);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);    
            }


            // ==========================================
            // END FRAME UI & SWAP
            // ==========================================
            
            RenderUI(engineContext);
    
            glfwPollEvents();
            glfwSwapBuffers(window);
}