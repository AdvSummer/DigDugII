#pragma once

#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <GL/glew.h>
#include <SDL_opengl.h>

class Shader
{
public:
    Shader(std::string name);
    ~Shader();

private:
    unsigned int program;

    unsigned int CompileShader(int type, std::string name);
};

