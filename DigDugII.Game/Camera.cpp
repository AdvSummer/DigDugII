#include "Camera.h"

Camera::Camera(glm::vec3 eye, glm::vec3 center)
    : eye(eye),
    center(center),
    up(glm::vec3(0.0, 1.0, 0.0))
{
}

Camera::~Camera()
{
}

glm::mat4 Camera::GetViewMatrix()
{
    return glm::lookAt(eye, center, up);
}
