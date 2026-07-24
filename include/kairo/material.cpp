#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include "kairo/material.h"


// MOSTLY VIBE CODED MATERIAL LOADER, NEED TO KEEP AN EYE ON IT

// Constructor
Material::Material(Shader* materialDefinition) 
{
    this->shader = materialDefinition;
}

// read json and store variables in ram
void Material::loadFromJson(const std::string& path) 
{
    if (path != "reload") { // if the path is "reload", instead of loading from a file, just reload the shader from the save path
        definitionPath = path;
    }
    
    std::ifstream file(definitionPath);
    if (!file.is_open()) {
        std::cout << "ERROR: Could not open material file: " << definitionPath << "\n";
        return;
    }

    nlohmann::json j = nlohmann::json::parse(file);

    int textureSlotCounter = 0;

    for (auto& [key, value] : j.items()) 
    {
        if (key.find("Path") != std::string::npos) 
        {
            std::string uniformName = key.substr(0, key.find("Path"));
            
            // create texture
            Texture* tex = new Texture(value.get<std::string>());
            
            // store texture and slot index in a struct
            textureBindings[uniformName] = { tex, textureSlotCounter };
            
            textureSlotCounter++;
        }
        // Handle Arrays (like "diffuseColor" : [1,0,0])
        else if (value.is_array() && value.size() == 3)
        {
            vec3s[key] = glm::vec3(value[0], value[1], value[2]);
        }
        // Handle Booleans (like "useDiffuseMap" : true)
        else if (value.is_boolean())
        {
            bools[key] = value.get<bool>();
        }
        // Handle Floats
        else if (value.is_number_float()) 
        {
            floats[key] = value.get<float>();
        }
        // Handle Ints
        else if (value.is_number_integer())
        {
            ints[key] = value.get<int>();
        }
    }
}


void Material::apply() 
{
    // need to activate the shader first so its the current one
    shader->use(); 

    // loop through all loaded textures and bind them
    for (auto const& [name, binding] : textureBindings) 
    {
        binding.tex->bind(binding.slot); // binding.slot as the index
        shader->setInt("material." + name, binding.slot);
    }

    // push the rest of the values to the shader
    for (auto& [name, val] : floats) { shader->setFloat("material." + name, val); }
    for (auto& [name, val] : ints)   { shader->setInt("material." + name, val); }
    for (auto& [name, val] : bools)  { shader->setBool("material." + name, val); } 
    for (auto& [name, val] : vec3s)  { shader->setVec3("material." + name, val); } 
}

Material::~Material() {
    for (auto const& [name, binding] : textureBindings) {
        delete binding.tex;
    }
}