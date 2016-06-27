#pragma once

#include <cmath>
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

    enum GameState
    {
        RUNNING,
        OVER,
        WIN
    };

    static const int LevelSize = 20;
    static const std::string LevelGroundPath;
    static const std::string LevelAbovePath;

    GameState state;
    SDL_DisplayMode display;
    SDL_Window *window;
    SDL_GLContext context;
    Shader *shader;
    Camera *mainCamera;
    Camera *camera;
    Camera *fpsCamera;
    Camera *thirdCamera;
    std::vector<Model*> models;
    std::vector<GameObject*> gameObjects;
    GameObject *levelGrid[2][LevelSize][LevelSize];
    GameObject *player;
    std::vector<GameObject*> enemies;

    void LoadModels();
    void LoadLevel();
    void MapImageToLevel(FIBITMAP * image, Level level);
    bool ExistsFloorAt(int x, int y);
    GameObject* GetGameObjectFromGrid(Level level, int x, int z);
    void AdjustBlocksTexture();
    void FloodFill();
    void Flood(GameObject* block, std::vector<GameObject*> *grassBlocks, std::vector<GameObject*> *area);
    void RemoveStrandedCracks();
    void HandleKeyboardInput(SDL_Keycode keyCode, SDL_EventType eventType);
    void CreateCrack();
    void PushEnemy();
    void Update();
    void CheckPlayerCollision();
    void UpdateEnemies();
    void UpdateCamera();
};
