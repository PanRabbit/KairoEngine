#pragma once

#include <vector>
#include <string>
#include "include/kairo/material.h"


struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec4 Tangent;
};

class Mesh {
    public:
        // mesh data
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        unsigned int materialSlot = 0;

        Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);
        void draw(Material &material);
        void drawShader(Shader& shader);
    private:
        // render data
        unsigned int VAO, VBO, EBO;

        void setupMesh();
};