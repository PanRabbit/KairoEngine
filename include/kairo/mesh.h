#pragma once

#include <vector>
#include <string>
#include "include/kairo/material.h"


struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

class Mesh {
    public:
        // mesh data
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);
        void draw(Material &material);
    private:
        // render data
        unsigned int VAO, VBO, EBO;

        void setupMesh();
};