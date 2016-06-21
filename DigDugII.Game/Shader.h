#pragma once

#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader
{
public:
    static const unsigned int PositionAttributeIndex = 0;
    static const unsigned int NormalAttributeIndex = 1;
    static const unsigned int TexCoordsAttributeIndex = 2;

    Shader(std::string name);
    ~Shader();

    unsigned int GetProgram();

    void SetUniform(std::string name, float value);
    void SetUniform(std::string name, glm::vec2 value);
    void SetUniform(std::string name, glm::vec3 value);
    void SetUniform(std::string name, glm::mat4 value);

private:
    unsigned int program;

    unsigned int CompileShader(int type, std::string name);
};

