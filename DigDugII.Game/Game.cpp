#include "Game.h"

const std::string Game::LevelGroundPath("../Resources/level/level_ground.png");
const std::string Game::LevelAbovePath("../Resources/level/level_above.png");

Game::Game()
    : levelGrid()
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_GetDesktopDisplayMode(0, &display);

    //window = SDL_CreateWindow("OpenGL", 100, 100, display.w, display.h, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
    window = SDL_CreateWindow("OpenGL", 100, 100, display.w, display.h, SDL_WINDOW_OPENGL);
    context = SDL_GL_CreateContext(window);

    glewExperimental = GL_TRUE;
    glewInit();

    glEnable(GL_DEPTH_TEST);

    shader = new Shader("shader");

    LoadModels();
    LoadLevel();

    camera = new Camera(glm::vec3(20.0, 40.0, 35.0), glm::vec3(20.0, 0.0, 20.0));
}

Game::~Game()
{
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Game::Run()
{
    SDL_Event windowEvent;
    while (true)
    {
        if (SDL_PollEvent(&windowEvent))
        {
            if (windowEvent.type == SDL_QUIT) break;
        }

        glClearColor(0.0f, 0.5f, 0.75f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Transformation matrices
        glm::mat4 projection = glm::perspective(45.0f, (float)display.w / display.h, 0.1f, 100.0f);
        //glm::mat4 projection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 0.0f, 50.0f);
        glm::mat4 view = camera->GetViewMatrix();
        shader->SetUniform("projection", projection);
        shader->SetUniform("view", view);

        // Set the lighting uniforms
        shader->SetUniform("viewPosition", camera->GetEye());
        shader->SetUniform("lights[0].position", glm::vec3(20.0, 20.0, 20.0));
        shader->SetUniform("lights[0].ambient", glm::vec3(0.3f, 0.3f, 0.3f));
        shader->SetUniform("lights[0].diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
        shader->SetUniform("lights[0].specular", glm::vec3(1.0f, 1.0f, 1.0f));
        shader->SetUniform("lights[0].constant", 1.0f);
        shader->SetUniform("lights[0].linear", 0.009f);
        shader->SetUniform("lights[0].quadratic", 0.0032f);

        //// Draw the loaded model
        //glm::mat4 model;
        //model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        //shader->SetUniform("model", model);
        //grassBlock->Draw(shader);

        //glm::mat4 model1;
        //model1 = glm::translate(model1, glm::vec3(2, 0.0f, 0));
        //shader->SetUniform("model", model1);
        //grassBlock->Draw(shader);

        for(GameObject *gameObject : gameObjects)
        {
            gameObject->Draw();
        }

        SDL_GL_SwapWindow(window);
    }
}

void Game::LoadModels()
{
    models.push_back(new Model("grass_block.obj"));
    models.push_back(new Model("hole_block.obj"));
    models.push_back(new Model("hole_one_block.obj"));
    models.push_back(new Model("hole_two_block.obj"));
    models.push_back(new Model("hole_two_l_block.obj"));
    models.push_back(new Model("hole_three_block.obj"));
    models.push_back(new Model("hole_four_block.obj"));
    models.push_back(new Model("crack_block.obj"));
    models.push_back(new Model("crack_one_block.obj"));
    models.push_back(new Model("crack_two_block.obj"));
    models.push_back(new Model("crack_two_l_block.obj"));
    models.push_back(new Model("crack_three_block.obj"));
    models.push_back(new Model("crack_four_block.obj"));
}

void Game::LoadLevel()
{
    int width;
    int height;
    FREE_IMAGE_FORMAT format;
    FIBITMAP *image;

    format = FreeImage_GetFileType(LevelGroundPath.c_str());

    image = FreeImage_Load(format, LevelGroundPath.c_str());
    width = FreeImage_GetWidth(image);
    height = FreeImage_GetHeight(image);

    if(width != LevelSize || height != LevelSize)
    {
        std::cerr << "GAME::LOAD_LEVEL::GROUND_WRONG_SIZE";
        return;
    }

    image = FreeImage_ConvertTo24Bits(image);
    MapImageToLevel(image, Level::GROUND);

    image = FreeImage_Load(format, LevelAbovePath.c_str());
    width = FreeImage_GetWidth(image);
    height = FreeImage_GetHeight(image);

    if(width != LevelSize || height != LevelSize)
    {
        std::cerr << "GAME::LOAD_LEVEL::ABOVE_WRONG_SIZE";
        return;
    }

    image = FreeImage_ConvertTo24Bits(image);
    MapImageToLevel(image, Level::ABOVE);
    AdjustBlocksTexture();
}

void Game::MapImageToLevel(FIBITMAP *image, Level level)
{
    for(int height = LevelSize - 1, i = 0; height >= 0; --height, ++i)
    {
        for(int width = 0; width < LevelSize; ++width)
        {
            GameObject *gameObject = nullptr;
            RGBQUAD color;

            FreeImage_GetPixelColor(image, width, height, &color);

            int hexColor = ((int)color.rgbRed << 16)
                + ((int)color.rgbGreen << 8)
                + (int)color.rgbBlue;

            switch(hexColor)
            {
            case 0x00ff00: // Green: normal terrain
                gameObject = new GameObject(shader, models[GameObject::GRASS], glm::vec3(width * 2.0, level * 2.0, i * 2.0), GameObject::GRASS);
                break;
            case 0x542100: // Dark Brown: hole
                if(ExistsFloorAt(width, height))
                {
                    GetGameObjectFromGrid(Level::GROUND, width, height)->SetModel(models[GameObject::HOLE], GameObject::HOLE);
                }
                break;
            case 0x7A5C46: // Light Brown: crack
                if(ExistsFloorAt(width, height))
                {
                    GetGameObjectFromGrid(Level::GROUND, width, height)->SetModel(models[GameObject::CRACK], GameObject::CRACK);
                }
                break;
            case 0xff0000: // Red: enemy
                if(ExistsFloorAt(width, height))
                {

                }
                break;
            case 0xffff00: // Yellow: player
                if(ExistsFloorAt(width, height))
                {

                }
                break;
            }

            if(gameObject != nullptr)
            {
                gameObjects.push_back(gameObject);
                levelGrid[level][height][width] = gameObject;
            }
        }
    }
}

bool Game::ExistsFloorAt(int x, int y)
{
    return GetGameObjectFromGrid(Level::GROUND, x, y) != nullptr;
}

GameObject * Game::GetGameObjectFromGrid(Level level, int x, int y)
{
    if(x < 0 || x >= LevelSize || y < 0 || y >= LevelSize)
    {
        return nullptr;
    }
    else
    {
        return levelGrid[level][y][x];
    }
}

void Game::AdjustBlocksTexture()
{
    for(int x = 0; x < LevelSize; ++x)
    {
        for(int y = 0; y < LevelSize; ++y)
        {
            GameObject *block = GetGameObjectFromGrid(Level::GROUND, x, y);
            if(block != nullptr && (block->GetObject() == GameObject::HOLE || block->GetObject() == GameObject::CRACK))
            {
                GameObject *top = GetGameObjectFromGrid(Level::GROUND, x, y + 1);
                GameObject *bottom = GetGameObjectFromGrid(Level::GROUND, x, y - 1);
                GameObject *right = GetGameObjectFromGrid(Level::GROUND, x + 1, y);
                GameObject *left = GetGameObjectFromGrid(Level::GROUND, x - 1, y);

                int neighbourCount = 0;
                std::vector<GameObject*> neighbours = {top, bottom, right, left};
                for(GameObject* neighbour : neighbours)
                {
                    if(neighbour == nullptr || neighbour->GetObject() == GameObject::HOLE || neighbour->GetObject() == GameObject::CRACK)
                    {
                        ++neighbourCount;
                    }
                }

                GameObject::Object object = GameObject::OBJECT_NULL;
                float degrees = 0.0;
                switch(neighbourCount)
                {
                case 0:
                    if(block->GetObject() == GameObject::HOLE)
                    {
                        object = GameObject::HOLE;
                    }
                    else
                    {
                        object = GameObject::CRACK;
                    }
                    break;
                case 1:
                    if(block->GetObject() == GameObject::HOLE)
                    {
                        object = GameObject::HOLE_ONE;
                    }
                    else
                    {
                        object = GameObject::CRACK_ONE;
                    }
                    if(left == nullptr || left->GetObject() == GameObject::HOLE || left->GetObject() == GameObject::CRACK)
                    {
                        degrees = 270.0;
                    }
                    else if(top == nullptr || top->GetObject() == GameObject::HOLE || top->GetObject() == GameObject::CRACK)
                    {
                        degrees = 180.0;
                    }
                    else if(right == nullptr || right->GetObject() == GameObject::HOLE || right->GetObject() == GameObject::CRACK)
                    {
                        degrees = 90.0;
                    }
                    break;
                case 2:
                    if(block->GetObject() == GameObject::HOLE)
                    {
                        object = GameObject::HOLE_TWO;
                    }
                    else
                    {
                        object = GameObject::CRACK_TWO;
                    }
                    if((right == nullptr || right->GetObject() == GameObject::HOLE || right->GetObject() == GameObject::CRACK) &&
                       (left == nullptr || left->GetObject() == GameObject::HOLE || left->GetObject() == GameObject::CRACK))
                    {
                        degrees = 90.0;
                        break;
                    }
                    else if((top == nullptr || top->GetObject() == GameObject::HOLE || top->GetObject() == GameObject::CRACK) &&
                            (bottom == nullptr || bottom->GetObject() == GameObject::HOLE || bottom->GetObject() == GameObject::CRACK))
                    {
                        break;
                    }
                    if(block->GetObject() == GameObject::HOLE)
                    {
                        object = GameObject::HOLE_TWO_L;
                    }
                    else
                    {
                        object = GameObject::CRACK_TWO_L;
                    }
                    if((bottom == nullptr || bottom->GetObject() == GameObject::HOLE || bottom->GetObject() == GameObject::CRACK) &&
                        (left == nullptr || left->GetObject() == GameObject::HOLE || left->GetObject() == GameObject::CRACK))
                    {
                        degrees = 270.0;
                    }
                    else if((left == nullptr || left->GetObject() == GameObject::HOLE || left->GetObject() == GameObject::CRACK) &&
                        (top == nullptr || top->GetObject() == GameObject::HOLE || top->GetObject() == GameObject::CRACK))
                    {
                        degrees = 180.0;
                    }
                    else if((right == nullptr || right->GetObject() == GameObject::HOLE || right->GetObject() == GameObject::CRACK) &&
                        (top == nullptr || top->GetObject() == GameObject::HOLE || top->GetObject() == GameObject::CRACK))
                    {
                        degrees = 90.0;
                    }
                    break;
                case 3:
                    if(block->GetObject() == GameObject::HOLE)
                    {
                        object = GameObject::HOLE_THREE;
                    }
                    else
                    {
                        object = GameObject::CRACK_THREE;
                    }
                    if(left != nullptr && left->GetObject() != GameObject::HOLE && left->GetObject() != GameObject::CRACK)
                    {
                        degrees = 90.0;
                    }
                    else if(bottom != nullptr && bottom->GetObject() != GameObject::HOLE && bottom->GetObject() != GameObject::CRACK)
                    {
                        degrees = 180.0;
                    }
                    else if(right != nullptr && right->GetObject() != GameObject::HOLE && right->GetObject() != GameObject::CRACK)
                    {
                        degrees = 270.0;
                    }
                    break;
                case 4:
                    if(block->GetObject() == GameObject::HOLE)
                    {
                        object = GameObject::HOLE_FOUR;
                    }
                    else
                    {
                        object = GameObject::CRACK_FOUR;
                    }
                    break;
                }

                if(object != GameObject::OBJECT_NULL)
                {
                    block->SetModel(models[object], block->GetObject());
                    block->Rotate(glm::radians(degrees));
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    Game *game = new Game();
    game->Run();

    return 0;
}
