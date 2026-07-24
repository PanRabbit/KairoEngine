#pragma once
#include <string>
#include <iostream>
#include <vector>
#include "include/kairo/material.h"
#include "include/kairo/mesh.h"

// Forward-declare Assimp structures so header stays lightweight
struct aiNode;
struct aiScene;
struct aiMesh;

class Model
{
    public:
       Model(const std::string& path)
       {
          loadModel(path);
       }
       void draw(Material &material);
   private:
       // model data
       std::vector<Mesh> meshes;
       std::string directory;
       
       void loadModel(std::string path);
       void processNode(aiNode *node, const aiScene *scene);
       Mesh processMesh(aiMesh *mesh, const aiScene *scene);
};