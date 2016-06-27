#include "GameObject.h"

GameObject::GameObject(Shader * shader, Model * model, glm::vec3 position, Object object, int positionX, int positionZ)
    : shader(shader),
    model(model),
    object(object),
    velocity(glm::vec3(0.0, 0.0, 0.0)),
    state(State::INERT),
    positionX(positionX),
    positionZ(positionZ),
    orientation(Orientation::DOWN)
{
    transformationMatrix = glm::translate(transformationMatrix, position);
}

GameObject::~GameObject()
{
}

void GameObject::Update()
{
    if(state == State::MOVING || state == State::PUSHED)
    {
        transformationMatrix = glm::translate(transformationMatrix, velocity);
    }
}

void GameObject::Draw()
{
    modelMatrix = GetModelMatrix();
    shader->SetUniform("model", modelMatrix);
    model->Draw(shader);
}

void GameObject::Rotate(float angle)
{
    rotationMatrix = glm::orientate4(glm::vec3(0.0, 0.0, glm::radians(angle)));
}

glm::mat4 GameObject::GetModelMatrix()
{
    return transformationMatrix * rotationMatrix * scaleMatrix;
}

GameObject::Object GameObject::GetObject()
{
    return this->object;
}

GameObject::Orientation GameObject::GetOrientation()
{
    return orientation;
}

GameObject::State GameObject::GetState()
{
    return state;
}

glm::vec3 GameObject::GetVelocity()
{
    return velocity;
}

int GameObject::GetPositionX()
{
    return positionX;
}

int GameObject::GetPositionZ()
{
    return positionZ;
}

int GameObject::GetTargetX()
{
    return targetX;
}

int GameObject::GetTargetZ()
{
    return targetZ;
}

void GameObject::SetModel(Model * model, Object object)
{
    this->model = model;
    this->object = object;
}

void GameObject::SetOrientation(Orientation orientation)
{
    this->orientation = orientation;
}

void GameObject::SetVelocity(glm::vec3 velocity)
{
    this->velocity = velocity;
}

void GameObject::SetScale(glm::vec3 scale)
{
    scaleMatrix = glm::scale(scaleMatrix, scale);
}

void GameObject::SetState(State state)
{
    this->state = state;
}

void GameObject::SetPositionX(int positionX)
{
    this->positionX = positionX;
}

void GameObject::SetPositionZ(int positionZ)
{
    this->positionZ = positionZ;
}

void GameObject::SetTargetX(int targetX)
{
    this->targetX = targetX;
}

void GameObject::SetTargetZ(int targetZ)
{
    this->targetZ = targetZ;
}
