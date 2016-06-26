#pragma once

#include <algorithm>
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "Model.h"
#include "GameObject.h"
#include "Terrain.h"
#include "Camera.h"

class Game
{
public:
    Game();
    ~Game();

    void Run();

private:
    enum Level
    {
        GROUND,
        ABOVE
    };

    static const int LevelSize = 20;
    static const std::string LevelGroundPath;
    static const std::string LevelAbovePath;

    SDL_DisplayMode display;
    SDL_Window *window;
    SDL_GLContext context;
    Shader *shader;
    Camera *camera;
    std::vector<Model*> models;
    std::vector<GameObject*> gameObjects;
    GameObject *levelGrid[2][LevelSize][LevelSize];
    GameObject *player;

    void LoadModels();
    void LoadLevel();
    void MapImageToLevel(FIBITMAP * image, Level level);
    bool ExistsFloorAt(int x, int y);
    GameObject* GetGameObjectFromGrid(Level level, int x, int y);
    void AdjustBlocksTexture();
    void FloodFill();
    void Flood(Terrain* block, std::vector<Terrain*>* grassBlocks, std::vector<Terrain*> *area);
    void HandleKeyboardInput(SDL_Keycode keyCode, SDL_EventType eventType);
};
