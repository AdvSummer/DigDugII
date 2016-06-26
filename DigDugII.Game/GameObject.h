#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
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
        PLAYER,
        OBJECT_NULL
    };

    enum State
    {
        INERT,
        MOVING
    };

    GameObject(Shader * shader, Model * model, glm::vec3 position, Object object);
    ~GameObject();

    void Update();
    void Draw();
    void Rotate(float angle);

    glm::mat4 GetModelMatrix();
    GameObject::Object GetObject();
    void SetModel(Model *model, Object object);
    void SetVelocity(glm::vec3 velocity);
    void SetState(State state);

private:
    Shader *shader;
    Model *model;
    glm::mat4 modelMatrix;
    glm::mat4 transformationMatrix;
    glm::mat4 rotationMatrix;
    glm::mat4 scaleMatrix;
    glm::vec3 velocity;
    Object object;
    State state;
};

