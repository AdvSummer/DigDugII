#pragma once

#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <GL/glew.h>

class Shader
{
public:
    Shader(std::string name);
    ~Shader();

    unsigned int getProgram();

private:
    unsigned int program;

    unsigned int CompileShader(int type, std::string name);
};

