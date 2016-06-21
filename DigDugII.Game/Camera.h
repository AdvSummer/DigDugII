#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
    Camera(glm::vec3 eye, glm::vec3 center);
    ~Camera();

    glm::vec3 GetEye();
    glm::mat4 GetViewMatrix();

private:
    glm::vec3 eye;
    glm::vec3 center;
    glm::vec3 up;
};

