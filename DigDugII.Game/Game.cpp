#include "Game.h"

const std::string Game::LevelGroundPath("../Resources/level/level_ground.png");
const std::string Game::LevelAbovePath("../Resources/level/level_above.png");

Game::Game()
    : levelGrid(),
    state(GameState::RUNNING)
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_GetDesktopDisplayMode(0, &display);

    window = SDL_CreateWindow("OpenGL", 100, 100, display.w, display.h, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
    context = SDL_GL_CreateContext(window);

    glewExperimental = GL_TRUE;
    glewInit();

    glEnable(GL_DEPTH_TEST);

    shader = new Shader("shader");

    LoadModels();
    LoadLevel();

    camera = new Camera(glm::vec3(20.0, 30.0, 30.0), glm::vec3(20.0, 0.0, 20.0), Camera::NORMAL);
    mainCamera = camera;
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
    bool running = true;
    while(running)
    {
        while(SDL_PollEvent(&windowEvent))
        {
            switch(windowEvent.type)
            {
            case SDL_QUIT:
                running = false;
                break;

            case SDL_KEYDOWN:
            case SDL_KEYUP:
                if(state == GameState::RUNNING)
                {
                    SDL_Keycode keyCode = windowEvent.key.keysym.sym;
                    HandleKeyboardInput(keyCode, (SDL_EventType)windowEvent.type);
                }
                break;
            }
        }
        
        if(state == GameState::RUNNING)
        {
            UpdateCamera();
            UpdateEnemies();
            Update();
        }

        glClearColor(0.0f, 0.5f, 0.75f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glViewport(0, 0, display.w, display.h);
        // Transformation matrices
        glm::mat4 projection = glm::perspective(45.0f, (float)display.w / display.h, 0.1f, 100.0f);
        glm::mat4 view = mainCamera->GetViewMatrix();
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

        for(GameObject *gameObject : gameObjects)
        {
            gameObject->Update();
            if(gameObject->GetObject() != GameObject::PLAYER || mainCamera->GetType() != Camera::FIRST_PERSON)
            {
                if(gameObject->GetObject() != GameObject::PLAYER || state != GameState::OVER)
                    gameObject->Draw();
            }
        }

        // minimap
        glViewport(0, display.h - (display.h * 0.2), display.w * 0.2, display.h * 0.2);
        projection = glm::ortho(-20.0, 20.0, -20.0, 20.0, 0.0, 50.0);
        view = camera->GetViewMatrix();
        shader->SetUniform("projection", projection);
        shader->SetUniform("view", view);

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
    models.push_back(new Model("player.obj"));
    models.push_back(new Model("enemy.obj"));
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
    FloodFill();
    RemoveStrandedCracks();
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
                gameObject = new GameObject(shader, models[GameObject::GRASS], glm::vec3(width * 2.0, level * 2.0, i * 2.0), GameObject::GRASS, width, i);
                break;
            case 0x542100: // Dark Brown: hole
                if(ExistsFloorAt(width, i))
                {
                    GetGameObjectFromGrid(Level::GROUND, width, i)->SetModel(models[GameObject::HOLE], GameObject::HOLE);
                }
                break;
            case 0x7A5C46: // Light Brown: crack
                if(ExistsFloorAt(width, i))
                {
                    GetGameObjectFromGrid(Level::GROUND, width, i)->SetModel(models[GameObject::CRACK], GameObject::CRACK);
                }
                break;
            case 0xff0000: // Red: enemy
                if(ExistsFloorAt(width, i))
                {
                    gameObject = new GameObject(shader, models[GameObject::ENEMY], glm::vec3(width * 2.0, 2.3, i * 2.0), GameObject::ENEMY, width, i);
                    enemies.push_back(gameObject);
                }
                break;
            case 0xffff00: // Yellow: player
                if(ExistsFloorAt(width, i))
                {
                    gameObject = new GameObject(shader, models[GameObject::PLAYER], glm::vec3(width * 2.0, 2.3, i * 2.0), GameObject::PLAYER, width, i);
                    fpsCamera = new Camera(glm::vec3(width * 2.0, 3.0, i * 2.0), glm::vec3(0.0, 0.0, 1.0), Camera::FIRST_PERSON);
                    thirdCamera = new Camera(glm::vec3(width * 2.0, 5.0, (i * 2.0) - 5.0), glm::vec3(width * 2.0, 2.0, i * 2.0), Camera::THIRD_PERSON);
                    player = gameObject;
                }
                break;
            }

            if(gameObject != nullptr)
            {
                gameObjects.push_back(gameObject);
                levelGrid[level][i][width] = gameObject;
            }
        }
    }
}

bool Game::ExistsFloorAt(int x, int y)
{
    return GetGameObjectFromGrid(Level::GROUND, x, y) != nullptr;
}

GameObject * Game::GetGameObjectFromGrid(Level level, int x, int z)
{
    if(x < 0 || x >= LevelSize || z < 0 || z >= LevelSize)
    {
        return nullptr;
    }
    else
    {
        return levelGrid[level][z][x];
    }
}

void Game::AdjustBlocksTexture()
{
    for(int x = 0; x < LevelSize; ++x)
    {
        for(int z = 0; z < LevelSize; ++z)
        {
            GameObject *block = GetGameObjectFromGrid(Level::GROUND, x, z);
            if(block != nullptr && (block->GetObject() == GameObject::HOLE || block->GetObject() == GameObject::CRACK))
            {
                GameObject *top = GetGameObjectFromGrid(Level::GROUND, x, z - 1);
                GameObject *bottom = GetGameObjectFromGrid(Level::GROUND, x, z + 1);
                GameObject *right = GetGameObjectFromGrid(Level::GROUND, x + 1, z);
                GameObject *left = GetGameObjectFromGrid(Level::GROUND, x - 1, z);

                int neighbourCount = 0;
                std::vector<GameObject*> neighbours = {top, bottom, right, left};
                for(GameObject* neighbour : neighbours)
                {
                    if(neighbour != nullptr && (neighbour->GetObject() == GameObject::HOLE || neighbour->GetObject() == GameObject::CRACK))
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
                    if(left != nullptr && (left->GetObject() == GameObject::HOLE || left->GetObject() == GameObject::CRACK))
                    {
                        degrees = 270.0;
                        if(right != nullptr)
                        {
                            break;
                        }
                    }
                    else if(top != nullptr && (top->GetObject() == GameObject::HOLE || top->GetObject() == GameObject::CRACK))
                    {
                        degrees = 180.0;
                        if(bottom != nullptr)
                        {
                            break;
                        }
                    }
                    else if(right != nullptr && (right->GetObject() == GameObject::HOLE || right->GetObject() == GameObject::CRACK))
                    {
                        degrees = 90.0;
                        if(left != nullptr)
                        {
                            break;
                        }
                    }
                    else
                    {
                        if(top != nullptr)
                        {
                            break;
                        }
                    }
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
                        if(left != nullptr)
                        {
                            break;
                        }
                    }
                    else if(bottom != nullptr && bottom->GetObject() != GameObject::HOLE && bottom->GetObject() != GameObject::CRACK)
                    {
                        degrees = 180.0;
                        if(bottom != nullptr)
                        {
                            break;
                        }
                    }
                    else if(right != nullptr && right->GetObject() != GameObject::HOLE && right->GetObject() != GameObject::CRACK)
                    {
                        degrees = 270.0;
                        if(right != nullptr)
                        {
                            break;
                        }
                    }
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
                    block->Rotate(degrees);
                }
            }
        }
    }
}

void Game::FloodFill()
{
    std::vector<GameObject*> grassBlocks;

    for(int x = 0; x < LevelSize; ++x)
    {
        for(int y = 0; y < LevelSize; ++y)
        {
            GameObject *block = GetGameObjectFromGrid(Level::GROUND, x, y);
            if(block != nullptr && block->GetObject() == GameObject::GRASS)
            {
                grassBlocks.push_back(block);
            }
        }
    }

    if(grassBlocks.empty())
    {
        return;
    }

    std::vector<GameObject*> firstArea;
    Flood(grassBlocks.front(), &grassBlocks, &firstArea);

    if(!grassBlocks.empty())
    {
        std::vector<GameObject*> secondArea;
        Flood(grassBlocks.front(), &grassBlocks, &secondArea);

        std::vector<GameObject*> *deleteArea;
        if(firstArea.size() <= secondArea.size())
        {
            deleteArea = &firstArea;
        }
        else
        {
            deleteArea = &secondArea;
        }

        while(!deleteArea->empty())
        {
            GameObject *block = deleteArea->front();

            block->SetState(GameObject::MOVING);
            block->SetVelocity(glm::vec3(0.0, -0.1, 0.0));

            int x = block->GetPositionX();
            int z = block->GetPositionZ();

            levelGrid[Level::GROUND][z][x] = nullptr;

            GameObject *above = GetGameObjectFromGrid(Level::ABOVE, x, z);
            if(above != nullptr && above->GetObject() == GameObject::GRASS)
            {
                above->SetState(GameObject::MOVING);
                above->SetVelocity(glm::vec3(0.0, -0.1, 0.0));

                levelGrid[Level::ABOVE][z][x] = nullptr;
            }

            deleteArea->erase(deleteArea->begin());
        }
    }
}

void Game::Flood(GameObject* block, std::vector<GameObject*> *grassBlocks, std::vector<GameObject*> *area)
{
    if(block == nullptr)
    {
        return;
    }

    // if block is not in the list, means the block was already visited or the block is not grass
    if(std::find(grassBlocks->begin(), grassBlocks->end(), block) == grassBlocks->end())
    {
        return;
    }

    int x = block->GetPositionX();
    int z = block->GetPositionZ();

    // block belongs to area
    area->push_back(block);

    // remove block from list
    grassBlocks->erase(std::remove(grassBlocks->begin(), grassBlocks->end(), block), grassBlocks->end());

    if(grassBlocks->empty())
    {
        return;
    }

    Flood(GetGameObjectFromGrid(Level::GROUND, x, z + 1), grassBlocks, area);
    Flood(GetGameObjectFromGrid(Level::GROUND, x, z - 1), grassBlocks, area);
    Flood(GetGameObjectFromGrid(Level::GROUND, x + 1, z), grassBlocks, area);
    Flood(GetGameObjectFromGrid(Level::GROUND, x - 1, z), grassBlocks, area);
}

void Game::RemoveStrandedCracks()
{
    std::vector<GameObject*> deleteArea;
    
    for(int x = 0; x < LevelSize; ++x)
    {
        for(int z = 0; z < LevelSize; ++z)
        {
            GameObject *block = GetGameObjectFromGrid(Level::GROUND, x, z);

            if(block != nullptr && (block->GetObject() == GameObject::CRACK || block->GetObject() == GameObject::HOLE))
            {
                GameObject *top = GetGameObjectFromGrid(Level::GROUND, x, z - 1);
                GameObject *bottom = GetGameObjectFromGrid(Level::GROUND, x, z + 1);
                GameObject *right = GetGameObjectFromGrid(Level::GROUND, x + 1, z);
                GameObject *left = GetGameObjectFromGrid(Level::GROUND, x - 1, z);
                GameObject *topRight = GetGameObjectFromGrid(Level::GROUND, x + 1, z - 1);
                GameObject *topLeft = GetGameObjectFromGrid(Level::GROUND, x - 1, z - 1);
                GameObject *bottomRight = GetGameObjectFromGrid(Level::GROUND, x + 1, z + 1);
                GameObject *bottomLeft = GetGameObjectFromGrid(Level::GROUND, x - 1, z + 1);

                std::vector<GameObject*> neighbours = {top, bottom, right, left, topRight, topLeft, bottomRight, bottomLeft};
                bool grassNearby = false;
                for(GameObject *neighbour : neighbours)
                {
                    if(neighbour != nullptr && neighbour->GetObject() == GameObject::GRASS)
                    {
                        grassNearby = true;
                        break;
                    }
                }

                if(!grassNearby)
                {
                    deleteArea.push_back(block);
                }
            }
        }
    }

    while(!deleteArea.empty())
    {
        GameObject *block = deleteArea.front();

        block->SetState(GameObject::MOVING);
        block->SetVelocity(glm::vec3(0.0, -0.1, 0.0));

        int x = block->GetPositionX();
        int z = block->GetPositionZ();

        levelGrid[Level::GROUND][z][x] = nullptr;

        GameObject *above = GetGameObjectFromGrid(Level::ABOVE, x, z);
        if(above != nullptr && above->GetObject() == GameObject::GRASS)
        {
            above->SetState(GameObject::MOVING);
            above->SetVelocity(glm::vec3(0.0, -0.1, 0.0));

            levelGrid[Level::ABOVE][z][x] = nullptr;
        }

        deleteArea.erase(deleteArea.begin());
    }
}

void Game::HandleKeyboardInput(SDL_Keycode keyCode, SDL_EventType eventType)
{
    switch(keyCode)
    {
	case SDLK_ESCAPE:
		SDL_Quit();
		break;
    case SDLK_w:
        player->Rotate(180.0);
        player->SetOrientation(GameObject::UP);
        if(eventType == SDL_KEYDOWN)
        {
            player->SetState(GameObject::MOVING);
            player->SetVelocity(glm::vec3(0.0, 0.0, -0.1));
        }
        else
        {
            player->SetState(GameObject::INERT);
        }
        break;
    case SDLK_a:
        player->Rotate(270.0);
        player->SetOrientation(GameObject::LEFT);
        if(eventType == SDL_KEYDOWN)
        {
            player->SetState(GameObject::MOVING);
            player->SetVelocity(glm::vec3(-0.1, 0.0, 0.0));
        }
        else
        {
            player->SetState(GameObject::INERT);
        }
        break;
    case SDLK_s:
        player->Rotate(0.0);
        player->SetOrientation(GameObject::DOWN);
        if(eventType == SDL_KEYDOWN)
        {
            player->SetState(GameObject::MOVING);
            player->SetVelocity(glm::vec3(0.0, 0.0, 0.1));
        }
        else
        {
            player->SetState(GameObject::INERT);
        }
        break;
    case SDLK_d:
        player->Rotate(90.0);
        player->SetOrientation(GameObject::RIGHT);
        if(eventType == SDL_KEYDOWN)
        {
            player->SetState(GameObject::MOVING);
            player->SetVelocity(glm::vec3(0.1, 0.0, 0.0));
        }
        else
        {
            player->SetState(GameObject::INERT);
        }
        break;
    case SDLK_SPACE:
        if(eventType == SDL_KEYDOWN)
        {
            GameObject *block = GetGameObjectFromGrid(Level::GROUND, player->GetPositionX(), player->GetPositionZ());

            if(block->GetObject() == GameObject::HOLE)
            {
                CreateCrack();
            }
        }
        break;
    case SDLK_f:
        if(eventType == SDL_KEYDOWN)
        {
            PushEnemy();
        }
        break;
    case SDLK_v:
        if(eventType == SDL_KEYDOWN)
        {
            if(mainCamera->GetType() == Camera::NORMAL)
            {
                mainCamera = fpsCamera;
            }
            else if(mainCamera->GetType() == Camera::FIRST_PERSON)
            {
                mainCamera = thirdCamera;
            }
            else
            {
                mainCamera = camera;
            }
        }
        break;
    }
}

void Game::CreateCrack()
{
    bool crack = false;
    std::vector<GameObject*> grassBlocks;

    int x = player->GetPositionX();
    int z = player->GetPositionZ();
    int ox;
    int oz;

    switch(player->GetOrientation())
    {
    case GameObject::UP:
        ox = 0;
        oz = -1;
        break;
    case GameObject::LEFT:
        ox = -1;
        oz = 0;
        break;
    case GameObject::DOWN:
        ox = 0;
        oz = 1;
        break;
    case GameObject::RIGHT:
        ox = 1;
        oz = 0;
        break;
    }

    GameObject *ground = nullptr;
    GameObject *above = nullptr;
    
    while(!crack && (above == nullptr || above->GetObject() != GameObject::GRASS))
    {
        x += ox;
        z += oz;
        
        ground = GetGameObjectFromGrid(Level::GROUND, x, z);
        above = GetGameObjectFromGrid(Level::ABOVE, x, z);

        if(ground == nullptr || ground->GetObject() == GameObject::HOLE || ground->GetObject() == GameObject::CRACK)
        {
            crack = true;
        }
        else if(ground->GetObject() == GameObject::GRASS)
        {
            grassBlocks.push_back(ground);
        }
    } 

    if(crack && !grassBlocks.empty())
    {
        for(GameObject* block : grassBlocks)
        {
            block->SetModel(models[GameObject::CRACK], GameObject::CRACK);
        }
        AdjustBlocksTexture();
        FloodFill();
        RemoveStrandedCracks();
    }
}

void Game::PushEnemy()
{
    int x = player->GetPositionX();
    int z = player->GetPositionZ();
    int ox;
    int oz;

    switch(player->GetOrientation())
    {
    case GameObject::UP:
        ox = 0;
        oz = -1;
        break;
    case GameObject::LEFT:
        ox = -1;
        oz = 0;
        break;
    case GameObject::DOWN:
        ox = 0;
        oz = 1;
        break;
    case GameObject::RIGHT:
        ox = 1;
        oz = 0;
        break;
    }

    GameObject *near = GetGameObjectFromGrid(Level::ABOVE, x + ox, z + oz);
    GameObject *far = GetGameObjectFromGrid(Level::ABOVE, x + (ox*2), z + (oz*2));

    if(near != nullptr && near->GetObject() == GameObject::ENEMY)
    {
        near->SetVelocity(glm::vec3((float)ox, 0.0, (float)oz));
        near->SetState(GameObject::PUSHED);
        int x = near->GetPositionX();
        int z = near->GetPositionZ();
        near->SetTargetX(x + (ox*2));
        near->SetTargetZ(z + (oz*2));
    }
    else if(far != nullptr && far->GetObject() == GameObject::ENEMY)
    {
        far->SetVelocity(glm::vec3((float)ox, 0.0, (float)oz));
        far->SetState(GameObject::PUSHED);
        int x = far->GetPositionX();
        int z = far->GetPositionZ();
        far->SetTargetX(x + (ox * 2));
        far->SetTargetZ(z + (oz * 2));
    }
}

void Game::Update()
{
    std::vector<GameObject*> objects;
    objects.assign(gameObjects.begin(), gameObjects.end());
    for(int i = 0; i < objects.size(); ++i)
    {
        glm::vec3 position(objects[i]->GetModelMatrix()[3]);

        if(position.y < -15.0 && objects[i]->GetObject() != GameObject::PLAYER)
        {
            gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), objects[i]), gameObjects.end());

            delete objects[i];
            objects[i] = nullptr;
        }
    }

    CheckPlayerCollision();

    std::vector<GameObject*> actors;
    actors.assign(enemies.begin(), enemies.end());
    actors.push_back(player);
    for(GameObject *actor : actors)
    {
        glm::vec3 position(actor->GetModelMatrix()[3]);
        int x = (int)std::round(position.x / 2.0);
        int z = (int)std::round(position.z / 2.0);

        // update pushed state
        if(actor->GetObject() == GameObject::ENEMY &&
           actor->GetState() == GameObject::PUSHED &&
           (x == actor->GetTargetX() && z == actor->GetTargetZ()))
        {
            actor->SetState(GameObject::INERT);
        }

        // update actor position
        if(x != actor->GetPositionX() ||
           z != actor->GetPositionZ())
        {
            GameObject *object = GetGameObjectFromGrid(Level::ABOVE, actor->GetPositionX(), actor->GetPositionZ());
            GameObject *above = GetGameObjectFromGrid(Level::ABOVE, x, z);

            if(above != nullptr &&
                ((actor->GetObject() == GameObject::PLAYER && above->GetObject() == GameObject::ENEMY) ||
               (actor->GetObject() == GameObject::ENEMY && above->GetObject() == GameObject::PLAYER)))
            {
                actor->SetState(GameObject::INERT);
                above->SetState(GameObject::INERT);
                state = GameState::OVER;
                break;
            }

            if(object != nullptr && (object->GetObject() == GameObject::PLAYER || object->GetObject() == GameObject::ENEMY))
            {
                levelGrid[Level::ABOVE][actor->GetPositionZ()][actor->GetPositionX()] = nullptr;
                levelGrid[Level::ABOVE][z][x] = actor;
                actor->SetPositionX(x);
                actor->SetPositionZ(z);
            }
        }
        // check if position is valid
        GameObject *ground = GetGameObjectFromGrid(Level::GROUND, x, z);
        if(ground == nullptr)
        {
            actor->SetVelocity(glm::vec3(0.0, -0.1, 0.0));
            actor->SetState(GameObject::MOVING);
            if(actor->GetObject() == GameObject::PLAYER)
            {
                state = GameState::OVER;
            }
            else if(actor->GetObject() == GameObject::ENEMY)
            {
                enemies.erase(std::remove(enemies.begin(), enemies.end(), actor), enemies.end());
            }
        }
    }

    if(state == GameState::OVER)
    {
        for(GameObject *actor : actors)
        {
            actor->SetState(GameObject::INERT);
        }
    }

    if(enemies.empty())
    {
        state = GameState::WIN;
        player->SetState(GameObject::INERT);
    }
}

void Game::CheckPlayerCollision()
{
    int x = player->GetPositionX();
    int z = player->GetPositionZ();

    GameObject *bottom = GetGameObjectFromGrid(Level::GROUND, x, z + 1);
    GameObject *bottomA = GetGameObjectFromGrid(Level::ABOVE, x, z + 1);
    GameObject *right = GetGameObjectFromGrid(Level::GROUND, x + 1, z);
    GameObject *rightA = GetGameObjectFromGrid(Level::ABOVE, x + 1, z);
    GameObject *top = GetGameObjectFromGrid(Level::GROUND, x, z - 1);
    GameObject *topA = GetGameObjectFromGrid(Level::ABOVE, x, z - 1);
    GameObject *left = GetGameObjectFromGrid(Level::GROUND, x - 1, z);
    GameObject *leftA = GetGameObjectFromGrid(Level::ABOVE, x - 1, z);

    if(player->GetState() == GameObject::MOVING)
    {
        glm::vec3 velocity(player->GetVelocity());

        if(velocity.x == 0.0 && velocity.z > 0.0)
        {
            if(bottomA != nullptr && bottomA->GetObject() == GameObject::GRASS)
            {
                player->SetState(GameObject::INERT);
            }
        }
        else if(velocity.x == 0.0 && velocity.z < 0.0)
        {
            if(topA != nullptr && topA->GetObject() == GameObject::GRASS)
            {
                player->SetState(GameObject::INERT);
            }
        }
        else if(velocity.x > 0.0 && velocity.z == 0.0)
        {
            if(rightA != nullptr && rightA->GetObject() == GameObject::GRASS)
            {
                player->SetState(GameObject::INERT);
            }
        }
        else if(velocity.x < 0.0 && velocity.z == 0.0)
        {
            if(leftA != nullptr && leftA->GetObject() == GameObject::GRASS)
            {
                player->SetState(GameObject::INERT);
            }
        }
    }
}

void Game::UpdateEnemies()
{
    for(GameObject *enemy : enemies)
    {
        int x = enemy->GetPositionX();
        int z = enemy->GetPositionZ();
            
        GameObject *bottom = GetGameObjectFromGrid(Level::GROUND, x, z + 1);
        GameObject *bottomA = GetGameObjectFromGrid(Level::ABOVE, x, z + 1);
        GameObject *right = GetGameObjectFromGrid(Level::GROUND, x + 1, z);
        GameObject *rightA = GetGameObjectFromGrid(Level::ABOVE, x + 1, z);
        GameObject *top = GetGameObjectFromGrid(Level::GROUND, x, z - 1);
        GameObject *topA = GetGameObjectFromGrid(Level::ABOVE, x, z - 1);
        GameObject *left = GetGameObjectFromGrid(Level::GROUND, x - 1, z);
        GameObject *leftA = GetGameObjectFromGrid(Level::ABOVE, x - 1, z);

        if(enemy->GetState() == GameObject::PUSHED)
        {
            glm::vec3 velocity(enemy->GetVelocity());

            if(velocity.x == 0.0 && velocity.z > 0.0)
            {
                if((bottom != nullptr && (bottom->GetObject() == GameObject::HOLE || bottom->GetObject() == GameObject::CRACK)) ||
                   (bottomA != nullptr && bottomA->GetObject() == GameObject::GRASS))
                {
                    enemy->SetState(GameObject::INERT);
                }
            }
            else if(velocity.x == 0.0 && velocity.z < 0.0)
            {
                if((top != nullptr && (top->GetObject() == GameObject::HOLE || top->GetObject() == GameObject::CRACK)) ||
                   (topA != nullptr && topA->GetObject() == GameObject::GRASS))
                {
                    enemy->SetState(GameObject::INERT);
                }
            }
            else if(velocity.x > 0.0 && velocity.z == 0.0)
            {
                if((right != nullptr && (right->GetObject() == GameObject::HOLE || right->GetObject() == GameObject::CRACK)) ||
                   (rightA != nullptr && rightA->GetObject() == GameObject::GRASS))
                {
                    enemy->SetState(GameObject::INERT);
                }
            }
            else if(velocity.x < 0.0 && velocity.z == 0.0)
            {
                if((left != nullptr && (left->GetObject() == GameObject::HOLE || left->GetObject() == GameObject::CRACK)) ||
                   (leftA != nullptr && leftA->GetObject() == GameObject::GRASS))
                {
                    enemy->SetState(GameObject::INERT);
                }
            }
        }
        else
        {

            std::vector<GameObject::Orientation> possibleActions;
            if(bottom != nullptr && bottom->GetObject() == GameObject::GRASS && (bottomA == nullptr || bottomA->GetObject() != GameObject::GRASS || bottomA->GetObject() != GameObject::ENEMY))
            {
                possibleActions.push_back(GameObject::DOWN);
            }
            if(right != nullptr && right->GetObject() == GameObject::GRASS && (rightA == nullptr || rightA->GetObject() != GameObject::GRASS || rightA->GetObject() != GameObject::ENEMY))
            {
                possibleActions.push_back(GameObject::RIGHT);
            }
            if(top != nullptr && top->GetObject() == GameObject::GRASS && (topA == nullptr || topA->GetObject() != GameObject::GRASS || topA->GetObject() != GameObject::ENEMY))
            {
                possibleActions.push_back(GameObject::UP);
            }
            if(left != nullptr && left->GetObject() == GameObject::GRASS && (leftA == nullptr || leftA->GetObject() != GameObject::GRASS || leftA->GetObject() != GameObject::ENEMY))
            {
                possibleActions.push_back(GameObject::LEFT);
            }
            
            if(!possibleActions.empty())
            {
                int action = -1;
                
                int playerX = player->GetPositionX();
                int playerZ = player->GetPositionZ();

                int distanceX = abs(x - playerX);
                int distanceZ = abs(z - playerZ);

                if(distanceX + distanceZ <= 4)
                {
                    if(distanceX != 0 && (distanceX <= distanceZ || distanceZ == 0))
                    {
                        if(x < playerX)
                        {
                            auto findAction = std::find(possibleActions.begin(), possibleActions.end(), GameObject::RIGHT);
                            if(findAction != possibleActions.end())
                            {
                                action = std::distance(possibleActions.begin(), findAction);
                            }
                        }
                        else
                        {
                            auto findAction = std::find(possibleActions.begin(), possibleActions.end(), GameObject::LEFT);
                            if(findAction != possibleActions.end())
                            {
                                action = std::distance(possibleActions.begin(), findAction);
                            }
                        }
                    }
                    else
                    {
                        if(z < playerZ)
                        {
                            auto findAction = std::find(possibleActions.begin(), possibleActions.end(), GameObject::DOWN);
                            if(findAction != possibleActions.end())
                            {
                                action = std::distance(possibleActions.begin(), findAction);
                            }
                        }
                        else
                        {
                            auto findAction = std::find(possibleActions.begin(), possibleActions.end(), GameObject::UP);
                            if(findAction != possibleActions.end())
                            {
                                action = std::distance(possibleActions.begin(), findAction);
                            }
                        }
                    }
                }
                
                if(action == -1)
                {
                    do
                    {
                        action = rand();
                    } while(action >= (RAND_MAX - (RAND_MAX % possibleActions.size())));
                    action %= possibleActions.size();
                }

                switch(possibleActions[action])
                {
                case GameObject::UP:
                    enemy->Rotate(180.0);
                    enemy->SetOrientation(GameObject::UP);
                    enemy->SetVelocity(glm::vec3(0.0, 0.0, -0.075));
                    enemy->SetState(GameObject::PUSHED);
                    enemy->SetTargetX(x);
                    enemy->SetTargetZ(z - 1);
                    break;
                case GameObject::LEFT:
                    enemy->Rotate(270.0);
                    enemy->SetOrientation(GameObject::LEFT);
                    enemy->SetVelocity(glm::vec3(-0.075, 0.0, 0.0));
                    enemy->SetState(GameObject::PUSHED);
                    enemy->SetTargetX(x - 1);
                    enemy->SetTargetZ(z);
                    break;
                case GameObject::DOWN:
                    enemy->Rotate(0.0);
                    enemy->SetOrientation(GameObject::DOWN);
                    enemy->SetVelocity(glm::vec3(0.0, 0.0, 0.075));
                    enemy->SetState(GameObject::PUSHED);
                    enemy->SetTargetX(x);
                    enemy->SetTargetZ(z + 1);
                    break;
                case GameObject::RIGHT:
                    enemy->Rotate(90.0);
                    enemy->SetOrientation(GameObject::RIGHT);
                    enemy->SetVelocity(glm::vec3(0.075, 0.0, 0.0));
                    enemy->SetState(GameObject::PUSHED);
                    enemy->SetTargetX(x + 1);
                    enemy->SetTargetZ(z);
                    break;
                }
            }
        }
    }
}

void Game::UpdateCamera()
{
    glm::vec3 position(player->GetModelMatrix()[3]);
    int ox;
    int oz;

    switch(player->GetOrientation())
    {
    case GameObject::UP:
        ox = 0;
        oz = -1;
        break;
    case GameObject::LEFT:
        ox = -1;
        oz = 0;
        break;
    case GameObject::DOWN:
        ox = 0;
        oz = 1;
        break;
    case GameObject::RIGHT:
        ox = 1;
        oz = 0;
        break;
    }
    
    if(mainCamera->GetType() == Camera::FIRST_PERSON)
    {
        mainCamera->Update(glm::vec3(position.x, 3.0, position.z), glm::vec3(ox, 0.0, oz));
    }
    else if(mainCamera->GetType() == Camera::THIRD_PERSON)
    {
        mainCamera->Update(glm::vec3(position.x - (ox*5.0), 5.0, position.z - (oz*5.0)), glm::vec3(position.x, 2.0, position.z));
    }
}

int main(int argc, char *argv[])
{
    Game *game = new Game();
    game->Run();

    return 0;
}
