#include "Shader.h"

Shader::Shader(std::string name)
{
    unsigned int vertexShader = CompileShader(GL_VERTEX_SHADER, name);
    unsigned int fragmentShader = CompileShader(GL_FRAGMENT_SHADER, name);

    program = glCreateProgram();

    glBindAttribLocation(program, 0, "position");
    glBindAttribLocation(program, 1, "color");
    glBindAttribLocation(program, 2, "texCoords");

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

unsigned int Shader::getProgram()
{
    return program;
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
