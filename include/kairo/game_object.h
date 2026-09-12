#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <kairo/model.h>
#include <kairo/material.h>
#include <kairo/shader.h>
#include <string>
#include <vector>

class GameObject {
public:
    static inline int nextID = 1;

    int id;
    std::string name;
    std::string modelName;
    std::vector<std::string> materialNames;
    Model* model;
    std::vector<Material*> materials;
    
    // Transform parameters (orientation is source of truth; euler is display/edit only)
    glm::vec3 position{0.0f};
    glm::quat orientation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f};

    GameObject(const std::string& name, Model* model, std::vector<Material*> materials,
               const std::string& modelName = "", std::vector<std::string> materialNames = {}):
        id(nextID++), name(name), modelName(modelName), materialNames(std::move(materialNames)),
        model(model), materials(std::move(materials)) {}

    void setMaterial(size_t slot, Material* mat, const std::string& materialName) {
        if (slot >= materials.size()) {
            materials.resize(slot + 1, nullptr);
            materialNames.resize(slot + 1);
        }
        materials[slot] = mat;
        materialNames[slot] = materialName;
    }
    // sync the object's material slots with the engine context
    void syncMaterialSlots(size_t slotCount, Material* fill, const std::string& fillName) {
        if (slotCount == 0)
            slotCount = 1;
        const size_t previous = materialNames.size();
        materialNames.resize(slotCount);
        materials.resize(slotCount, nullptr);
        for (size_t i = 0; i < slotCount; ++i) {
            if (i >= previous || materialNames[i].empty())
                materialNames[i] = fillName;
            if (!materials[i])
                materials[i] = fill;
        }
    }

    // Matches the old T * Rx * Ry * Rz * S compose used by level JSON
    void setEulerXYZ(const glm::vec3& radians) {
        glm::mat4 R(1.0f);
        R = glm::rotate(R, radians.x, glm::vec3(1.0f, 0.0f, 0.0f));
        R = glm::rotate(R, radians.y, glm::vec3(0.0f, 1.0f, 0.0f));
        R = glm::rotate(R, radians.z, glm::vec3(0.0f, 0.0f, 1.0f));
        orientation = glm::normalize(glm::quat_cast(R));
    }

    glm::vec3 getEulerXYZ() const {
        glm::mat3 R = glm::mat3_cast(orientation);
        glm::vec3 euler(0.0f);
        euler.y = glm::asin(glm::clamp(R[2][0], -1.0f, 1.0f));
        const float cy = glm::cos(euler.y);
        if (glm::abs(cy) > 1e-6f) {
            euler.x = glm::atan(-R[2][1], R[2][2]);
            euler.z = glm::atan(-R[1][0], R[0][0]);
        } else {
            euler.x = glm::atan(R[1][2], R[1][1]);
            euler.z = 0.0f;
        }
        return euler;
    }

    void setFromMatrix(const glm::mat4& model) {
        position = glm::vec3(model[3]);

        glm::vec3 col0(model[0]);
        glm::vec3 col1(model[1]);
        glm::vec3 col2(model[2]);
        scale = glm::vec3(glm::length(col0), glm::length(col1), glm::length(col2));

        const float eps = 1e-8f;
        glm::mat3 rot(
            scale.x > eps ? col0 / scale.x : glm::vec3(1.0f, 0.0f, 0.0f),
            scale.y > eps ? col1 / scale.y : glm::vec3(0.0f, 1.0f, 0.0f),
            scale.z > eps ? col2 / scale.z : glm::vec3(0.0f, 0.0f, 1.0f)
        );
        if (glm::determinant(rot) < 0.0f) {
            scale.x = -scale.x;
            rot[0] = -rot[0];
        }
        orientation = glm::normalize(glm::quat_cast(rot));
    }

    // compute model transformation matrix
    glm::mat4 getTransformMatrix() const {
        return glm::translate(glm::mat4(1.0f), position)
             * glm::mat4_cast(orientation)
             * glm::scale(glm::mat4(1.0f), scale);
    }

    // selection pass
    void drawSelection(Shader& selectionShader) const {
        selectionShader.setInt("objectID", id);
        selectionShader.setMat4("model", getTransformMatrix());
        model->drawShader(selectionShader);
    }

    // rendering pass
    void draw(Shader& shader, int selectedID) const {
        shader.use();
        shader.setMat4("model", getTransformMatrix());
        shader.setBool("isSelected", id == selectedID);
        model->draw(materials);
    }

    void drawShader(Shader& shader) const {
        shader.use();
        shader.setMat4("model", getTransformMatrix());
        model->drawShader(shader);
    }
};
