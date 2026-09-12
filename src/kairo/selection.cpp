#include "kairo/selection.h"
#include "kairo/engine_context.h"
#include "kairo/game_object.h"
#include "kairo/shader.h"
#include "kairo/model.h"
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

void SelectionBuffer::init(int width, int height) { 
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Color Texture (Stores IDs)
    glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

    // Depth Buffer (Needed so overlapping geometry occludes properly!)
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SelectionBuffer::cleanup() {
    if (fbo) glDeleteFramebuffers(1, &fbo);
    if (colorTexture) glDeleteTextures(1, &colorTexture);
    if (depthBuffer) glDeleteRenderbuffers(1, &depthBuffer);
}

// draw a sphere to select a point light, spot light, or sun
static void DrawSelectableSphere(Shader& selectionShader, Model& sphere, const glm::vec3& position, int id, float scale = 0.2f)
{
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model = glm::scale(model, glm::vec3(scale));
    selectionShader.setInt("objectID", id);
    selectionShader.setMat4("model", model);
    sphere.drawShader(selectionShader);
}

int PerformSelection(double mouseX, double mouseY, int screenWidth, int screenHeight,
    Shader& selectionShader, SelectionBuffer& selectionFB, EngineContext& engineContext,
    const glm::mat4& view, const glm::mat4& projection)
{
    // Bind our offscreen framebuffer so we render to a texture instead of the screen
    glBindFramebuffer(GL_FRAMEBUFFER, selectionFB.fbo);
    glViewport(0, 0, screenWidth, screenHeight);
    glEnable(GL_DEPTH_TEST);

    // Clear to black (ID = 0 means "nothing was clicked")
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up the selection shader with camera matrices
    selectionShader.use();
    selectionShader.setMat4("view", view);
    selectionShader.setMat4("projection", projection);

    // Render every object — each one writes its own ID into the fragment color
    for (const auto& [name, obj] : engineContext.sceneObjects) {
        obj->drawSelection(selectionShader);
    }
    // draw invisible spheres for point lights, spot lights, and sun
    auto sphereIt = engineContext.models.find("sphere");
    if (engineContext.showLightSpheres && sphereIt != engineContext.models.end()) {
        Model& sphere = *sphereIt->second;
        for (int i = 0; i < static_cast<int>(engineContext.pointLightPositions.size()); ++i) {
            DrawSelectableSphere(selectionShader, sphere, engineContext.pointLightPositions[i],
                                 EngineContext::PointLightSelectID(i));
        }
        for (int i = 1; i < static_cast<int>(engineContext.spotLightPositions.size()); ++i) {
            DrawSelectableSphere(selectionShader, sphere, engineContext.spotLightPositions[i],
                                 EngineContext::SpotLightSelectID(i));
        }
        DrawSelectableSphere(selectionShader, sphere, engineContext.sunHandlePosition(),
                             EngineContext::SUN_SELECT_ID, 0.35f);
    }

    // Read the single pixel under the mouse. OpenGL Y is flipped relative to GLFW, so we flip it back.
    int flippedY = screenHeight - static_cast<int>(mouseY);
    unsigned char pixel[3];
    glReadPixels(static_cast<int>(mouseX), flippedY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);

    // Unbind the FBO so rendering goes back to the screen normally
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Decode the RGB color back into a single integer ID (R + G*256 + B*65536)
    return pixel[0] + (pixel[1] * 256) + (pixel[2] * 256 * 256);
}
