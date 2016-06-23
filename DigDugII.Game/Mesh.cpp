#include "Mesh.h"

Mesh::Mesh(std::vector<Vertex> vertices,
           std::vector<unsigned int> indices,
           std::vector<Texture> textures)
    : shininess(16.0f)
{
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);
    glGenBuffers(1, &this->EBO);

    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex),
                 &this->vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(unsigned int),
                 &this->indices[0], GL_STATIC_DRAW);

    // Vertex Positions
    glEnableVertexAttribArray(Shader::PositionAttributeIndex);
    glVertexAttribPointer(Shader::PositionAttributeIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // Vertex Normals
    glEnableVertexAttribArray(Shader::NormalAttributeIndex);
    glVertexAttribPointer(Shader::NormalAttributeIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    // Vertex Texture Coords
    glEnableVertexAttribArray(Shader::TexCoordsAttributeIndex);
    glVertexAttribPointer(Shader::TexCoordsAttributeIndex, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

    glBindVertexArray(0);
}

Mesh::~Mesh()
{
}

void Mesh::Draw(Shader *shader)
{
    unsigned int diffuseCount = 1;
    unsigned int specularCount = 1;
    for (unsigned int i = 0; i < this->textures.size(); ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);

        std::stringstream ss;
        std::string number;
        std::string name = this->textures[i].type;
        if(name == "texture_diffuse")
        {
            ss << diffuseCount++;
        }
        else if(name == "texture_specular")
        {
            ss << specularCount++;
        }
        number = ss.str();

        std::string material = "material." + name + number;
        shader->SetUniform(material.c_str(), i);

        glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
    }

    shader->SetUniform("material.shininess", shininess);

    // Draw mesh
    glBindVertexArray(this->VAO);
    glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    for(unsigned int i = 0; i < this->textures.size(); ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}
