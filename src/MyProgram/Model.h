#pragma once

#include <gl/gl3w.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>
#include <string>
#include <unordered_map>

#include "Mesh.h"

class Model
{
public:
    Model(const std::string& path);
    ~Model();

public:
    void Draw(GLuint shaderId);

private:
    void ProcessNodeRecursive(aiNode* node, const aiScene* scene);
    Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

private:
    std::vector<Mesh> mMeshes;
    std::string directory;
    std::vector<Texture> mTexturesLoaded;
    std::unordered_map<std::string, Texture> mLoadedTextures;
};