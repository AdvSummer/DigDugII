#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"
#include "Model.h"

class GameObject
{
public:
    enum Object
    {
        GRASS,
        HOLE,
        HOLE_ONE,
        HOLE_TWO,
        HOLE_TWO_L,
        HOLE_THREE,
        HOLE_FOUR,
        CRACK,
        CRACK_ONE,
        CRACK_TWO,
        CRACK_TWO_L,
        CRACK_THREE,
        CRACK_FOUR,
        OBJECT_NULL
    };

    GameObject(Shader * shader, Model * model, glm::vec3 position, Object object);
    ~GameObject();

    void Draw();
    void Rotate(float angle);

    GameObject::Object GetObject();
    void SetModel(Model *model, Object object);

private:
    Shader *shader;
    Model *model;
    glm::mat4 modelMatrix;
    Object object;
};

