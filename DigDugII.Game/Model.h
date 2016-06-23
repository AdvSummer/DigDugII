#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <FreeImage.h>
#include "Mesh.h"

class Model
{
public:
    Model(std::string name);
    ~Model();

    void Draw(Shader *shader);

private:
    static const std::string modelDir;

    std::string modelName;
    std::vector<Mesh> meshes;
    std::vector<Texture> textures;

    void loadModel();
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
    unsigned int loadTexture(std::string path);
};

