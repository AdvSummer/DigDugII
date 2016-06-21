#include "Shader.h"

Shader::Shader(std::string name)
{
    unsigned int vertexShader = CompileShader(GL_VERTEX_SHADER, name);
    unsigned int fragmentShader = CompileShader(GL_FRAGMENT_SHADER, name);

    program = glCreateProgram();

    glBindAttribLocation(program, PositionAttributeIndex, "position");
    glBindAttribLocation(program, NormalAttributeIndex, "normal");
    glBindAttribLocation(program, TexCoordsAttributeIndex, "texCoords");

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE)
    {
        int length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        std::string log(length, ' ');
        glGetProgramInfoLog(program, length, &length, &log[0]);
        std::cerr << "ERROR::SHADER::LINKING_FAILED\n" << log << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glUseProgram(program);
}


Shader::~Shader()
{
    glDeleteProgram(program);
}

unsigned int Shader::GetProgram()
{
    return program;
}

void Shader::SetUniform(std::string name, float value)
{
    int location = glGetUniformLocation(program, name.c_str());
    glUniform1f(location, value);
}

void Shader::SetUniform(std::string name, glm::vec2 value)
{
    int location = glGetUniformLocation(program, name.c_str());
    glUniform2f(location, value.x, value.y);
}

void Shader::SetUniform(std::string name, glm::vec3 value)
{
    int location = glGetUniformLocation(program, name.c_str());
    glUniform3f(location, value.x, value.y, value.z);
}

void Shader::SetUniform(std::string name, glm::mat4 value)
{
    int location = glGetUniformLocation(program, name.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

unsigned int Shader::CompileShader(int type, std::string name)
{
    std::string ext(type == GL_VERTEX_SHADER ? "vert" : "frag");
    std::string filename("../glsl/" + name + "." + ext);
    std::ifstream fileStream(filename);
    std::string codeFile((std::istreambuf_iterator<char>(fileStream)),
                          std::istreambuf_iterator<char>());

    unsigned int shader = glCreateShader(type);

    const char *code = codeFile.c_str();
    glShaderSource(shader, 1, &code, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        int length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::string log(length, ' ');
        glGetShaderInfoLog(shader, length, &length, &log[0]);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << log << std::endl;
    }

    return shader;
}
