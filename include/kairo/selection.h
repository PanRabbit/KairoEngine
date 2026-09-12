#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <kairo/shader.h>
#include <kairo/game_object.h>

struct SelectionBuffer {
    unsigned int fbo = 0;
    unsigned int colorTexture = 0;
    unsigned int depthBuffer = 0;

    void init(int width, int height);
    void cleanup();
};

struct EngineContext;

int PerformSelection(double mouseX, double mouseY, int screenWidth, int screenHeight,
    Shader& selectionShader, SelectionBuffer& selectionFB, EngineContext& engineContext,
    const glm::mat4& view, const glm::mat4& projection); 