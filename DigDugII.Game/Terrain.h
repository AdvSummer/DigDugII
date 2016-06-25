#pragma once

#include "GameObject.h"

class Terrain : public GameObject
{
public:
    Terrain(Shader * shader, Model * model, glm::vec3 position, Object object, int positionX, int positionY);
    ~Terrain();

    int GetPositionX();
    int GetPositionY();

private:
    int positionX;
    int positionY;
};

