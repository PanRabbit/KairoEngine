#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <kairo/model.h>
#include <kairo/material.h>
#include <kairo/shader.h>

class GameObject {
public:
    static inline int nextID = 1;

    int id;
    std::string name;
    Model* model;
    Material* material;
    
    // Transform parameters
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};
    glm::vec3 scale{1.0f};

    GameObject(const std::string& name, Model* model, Material*material):
        id (nextID++), name(name), model(model), material(material) {}

    // compute model transformation matrix
    glm::mat4 getTransformMatrix() const {
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, position);
        transform = glm::rotate(transform, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        transform = glm::rotate(transform, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        transform = glm::rotate(transform, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        transform = glm::scale(transform, scale);
        return transform;
    }

    // selection pass
    void drawSelection(Shader& selectionShader) const {
        selectionShader.setInt("objectID", id);
        selectionShader.setMat4("model", getTransformMatrix());
        model->drawShader(selectionShader);
    }

    // rendering pass
    void draw(Shader& shader, int selectedID) const {
        material->apply();
        shader.setMat4("model", getTransformMatrix());
        shader.setBool("isSelected", id == selectedID);

        model->draw(*material);
    }
};