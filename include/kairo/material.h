#pragma once
#include <kairo/shader.h>
#include <kairo/texture.h>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

struct TextureBinding {
    Texture* tex;
    int slot;
};

class Material {
    public:
        Shader* shader;
        
        std::string definitionPath = "";

        // Dynamic variables
        std::unordered_map<std::string, TextureBinding> textureBindings;
        std::unordered_map<std::string, float> floats;
        std::unordered_map<std::string, int> ints;
        std::unordered_map<std::string, bool> bools;
        std::unordered_map<std::string, glm::vec3> vec3s;

        //std::unordered_map<std::string, glm::mat4> mat4s; //transformations
        //std::unordered_map<std::string, glm::vec4> vec4s;
        //std::unordered_map<std::string, glm::vec2> vec2s;

        Material(Shader* materialDefinition);

        ~Material();

        void  loadFromJson(const std::string& path);
        void apply();
};
