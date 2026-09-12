#pragma once
#include <string>
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
       void draw(const std::vector<Material*>& materials);
       void drawShader(Shader& shader);

       size_t materialSlotCount() const { return slotNames.empty() ? 1 : slotNames.size(); }
       const std::string& slotName(size_t slot) const;

   private:
       // model data
       std::vector<Mesh> meshes;
       std::vector<std::string> slotNames;
       
       void loadModel(std::string path);
       void processNode(aiNode *node, const aiScene *scene);
       Mesh processMesh(aiMesh *mesh);
       void compactMaterialSlots();
};