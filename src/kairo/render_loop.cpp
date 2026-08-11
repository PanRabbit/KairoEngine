#include "kairo/engine_context.h"
#include "kairo/shader.h"

float lastFrame = 0.0f;

void RenderLoop(EngineContext& engineContext, const std::vector<Shader*>& engineShaders, const std::vector<Material*>& engineMaterials) {
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
    
            // ==========================================
            // LIGHTS AND CAMERA VARIABLES
            // ==========================================
            Shader* phongShader = engineShaders[0];
            Shader* lightShader = engineShaders[1];
            Material* lightMaterial = engineMaterials[4];   
            
            phongShader->use();
    
            // Directional light (sun)
            glm::vec3 sunColor = glm::vec3(engineContext.clearColor[0], engineContext.clearColor[1], engineContext.clearColor[2]);
            phongShader->setVec3("dirLight.direction", sunDirection);
            phongShader->setFloat("dirLight.intensity", 5.0f);
            phongShader->setVec3("dirLight.ambient", sunColor * 0.15f);
            phongShader->setVec3("dirLight.diffuse", sunColor * 1.0f);
            phongShader->setVec3("dirLight.specular", sunColor * 1.0f);
    
            // Point lights
            for(unsigned int i = 0; i < 3; i++)
            {
                lightColor = pointLightColors[i];
                std::string uniformID = "pointLights[" + std::to_string(i) + "].";
                phongShader->setVec3(uniformID + "ambient", lightColor * 0.15f);
                phongShader->setVec3(uniformID + "diffuse", lightColor * 1.0f);
                phongShader->setVec3(uniformID + "specular", lightColor * 1.0f);
                phongShader->setVec3(uniformID + "position", pointLightPositions[i]);
                phongShader->setFloat(uniformID + "radius", 8.0f);
                phongShader->setFloat(uniformID + "intensity", 0.5f * pointLightIntensityMults[i]);
            }
    
            // Flashlight
            if (flashlightOn)
            {
                phongShader->setVec3("spotLight.position", camera.Position);
                phongShader->setVec3("spotLight.direction", camera.Front);
                phongShader->setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
                phongShader->setFloat("spotLight.outerCutOff", glm::cos(glm::radians(20.0f)));
                phongShader->setVec3("spotLight.ambient", torchColor * 0.15f);
                phongShader->setVec3("spotLight.diffuse", torchColor * 1.0f);
                phongShader->setVec3("spotLight.specular", torchColor * 1.0f);
                phongShader->setFloat("spotLight.radius", 64.0f);
                phongShader->setFloat("spotLight.intensity", 32.0f);
            }
            else
            {
                phongShader->setFloat("spotLight.cutOff", glm::cos(glm::radians(0.0f)));
                phongShader->setFloat("spotLight.outerCutOff", glm::cos(glm::radians(0.0f)));
                phongShader->setFloat("spotLight.intensity", 0.0f);
            }
    
            // Camera matrices
            phongShader->setMat4("view", view);
            phongShader->setMat4("projection", projection);
            phongShader->setVec3("viewPos", camera.Position);
    
            // ==========================================
            // BEGIN DRAW
            // ==========================================
            glClearColor(sunColor.r, sunColor.g, sunColor.b, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
            // Render pass — just iterate objects directly:
            for (auto* obj : sceneObjects) {
                obj->draw(phongShader, engineContext.selectedObjectID);
            }
    
            // DRAW LIGHT SPHERES
            lightShader.use();
            lightShader.setMat4("view", view);
            lightShader.setMat4("projection", projection);
    
            for(unsigned int i = 0; i < 3; i++)
            {   
                lightColor = pointLightColors[i];
                lightShader.setVec3("Color", lightColor);
                glm::mat4 lightModel = glm::mat4(1.0f); 
                lightModel = glm::translate(lightModel, pointLightPositions[i]); 
                lightModel = glm::scale(lightModel, glm::vec3(0.2f)); 
                lightShader.setMat4("model", lightModel);
                meshSphere.draw(lightMaterial);
            }
    
            // ==========================================
            // END FRAME UI & SWAP
            // ==========================================
    
            engineContext.deltaTime = deltaTime;
            engineContext.scrWidth = screenWidth;
            engineContext.scrHeight = screenHeight;
            engineContext.flashlightOn = flashlightOn;
    
            
            engineContext.cameraSpeed = camera.MovementSpeed;
            engineContext.cameraPos = camera.Position;
            engineContext.cameraRot = camera.Front;
            engineContext.fov = camera.Zoom;
            
            processInput(window, engineContext);
            RenderUI(engineContext, allShaders, allMaterials);
    
            glfwPollEvents();
            glfwSwapBuffers(window);
}