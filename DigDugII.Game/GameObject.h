#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
//#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
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
        ENEMY,
        OBJECT_NULL
    };

    enum State
    {
        INERT,
        MOVING,
        PUSHED
    };

    enum Orientation
    {
        DOWN,
        RIGHT,
        UP,
        LEFT
    };

    GameObject(Shader * shader, Model * model, glm::vec3 position, Object object, int positionX, int positionY);
    ~GameObject();

    void Update();
    void Draw();
    void Rotate(float angle);

    glm::mat4 GetModelMatrix();
    GameObject::Object GetObject();
    GameObject::Orientation GetOrientation();
    GameObject::State GetState();
    glm::vec3 GetVelocity();
    int GetPositionX();
    int GetPositionZ();
    int GetTargetX();
    int GetTargetZ();

    void SetModel(Model *model, Object object);
    void SetOrientation(Orientation orientation);
    void SetVelocity(glm::vec3 velocity);
    void SetScale(glm::vec3 scale);
    void SetState(State state);
    void SetPositionX(int positionX);
    void SetPositionZ(int positionZ);
    void SetTargetX(int targetX);
    void SetTargetZ(int targetZ);

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
    Orientation orientation;
    int positionX;
    int positionZ;
    int targetX;
    int targetZ;
};

