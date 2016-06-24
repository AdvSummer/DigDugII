#include "GameObject.h"

GameObject::GameObject(Shader * shader, Model * model, glm::vec3 position, Object object)
    : shader(shader),
    model(model),
    object(object)
{
    modelMatrix = glm::translate(modelMatrix, position);
}

GameObject::~GameObject()
{
}

void GameObject::Draw()
{
    shader->SetUniform("model", modelMatrix);
    model->Draw(shader);
}

void GameObject::Rotate(float angle)
{
    modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0.0, 1.0, 0.0));
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
