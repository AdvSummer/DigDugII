#include "Camera.h"

Camera::Camera(glm::vec3 eye, glm::vec3 center, Type type)
    : eye(eye),
    center(center),
    up(glm::vec3(0.0, 1.0, 0.0)),
    type(type)
{
}

Camera::~Camera()
{
}

glm::vec3 Camera::GetEye()
{
    return eye;
}

glm::mat4 Camera::GetViewMatrix()
{
    if(type == Type::FIRST_PERSON)
    {
        return glm::lookAt(eye, eye + center, up);
    }
    else
    {
        return glm::lookAt(eye, center, up);
    }
}

Camera::Type Camera::GetType()
{
    return type;
}

void Camera::Update(glm::vec3 eye, glm::vec3 center)
{
    this->eye = eye;
    this->center = center;
}
