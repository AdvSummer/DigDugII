#include "GameObject.h"

GameObject::GameObject(Shader * shader, Model * model, glm::vec3 position, Object object)
    : shader(shader),
    model(model),
    object(object),
    velocity(glm::vec3(0.0, 0.0, 0.0)),
    state(State::INERT)
{
    transformationMatrix = glm::translate(transformationMatrix, position);
}

GameObject::~GameObject()
{
}

void GameObject::Update()
{
    if(state == State::MOVING)
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

void GameObject::SetModel(Model * model, Object object)
{
    this->model = model;
    this->object = object;
}

void GameObject::SetVelocity(glm::vec3 velocity)
{
    this->velocity = velocity;
}

void GameObject::SetState(State state)
{
    this->state = state;
}
