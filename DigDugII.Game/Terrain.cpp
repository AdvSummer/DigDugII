#include "Terrain.h"

Terrain::Terrain(Shader * shader, Model * model, glm::vec3 position, Object object, int positionX, int positionY)
    : GameObject(shader, model, position, object),
    positionX(positionX),
    positionY(positionY)
{
}

Terrain::~Terrain()
{
}

int Terrain::GetPositionX()
{
    return positionX;
}

int Terrain::GetPositionY()
{
    return positionY;
}
