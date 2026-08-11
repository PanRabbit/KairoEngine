#pragma once

#include <vector>
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

int PerformSelection(double mouseX, double mouseY, int screenWidth, int screenHeight, 
    Shader& selectionShader, SelectionBuffer& selectionFB, const std::vector<std::unique_ptr<GameObject>>& sceneObjects,
    const glm::mat4& view, const glm::mat4& projection); 