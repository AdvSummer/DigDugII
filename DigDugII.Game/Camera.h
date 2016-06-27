#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
    enum Type
    {
        NORMAL,
        FIRST_PERSON,
        THIRD_PERSON
    };

    Camera(glm::vec3 eye, glm::vec3 center, Type type);
    ~Camera();

    glm::vec3 GetEye();
    glm::mat4 GetViewMatrix();
    Camera::Type GetType();

    void Update(glm::vec3 eye, glm::vec3 center);

private:
    glm::vec3 eye;
    glm::vec3 center;
    glm::vec3 up;
    Type type;
};

