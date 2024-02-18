#include "Model.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>

#include <fmt/core.h>

#include <string>
#include <vector>
#include <cstring>
#include <utility>
#include <unordered_map>

#include "helper.h"

Model::Model(const std::string& path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate |
            aiProcess_FlipUVs);

    if (!scene ||
        !scene->mRootNode ||
        scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
    {
        fmt::print(stderr, "[ASSIMP-ERROR] {}\n", importer.GetErrorString());
        return;
    }
    fmt::print("[ASSIMP] Successfully loaded \"{}\"\n", path);
    directory = path.substr(0, path.find_last_of('/'));

    ProcessNodeRecursive(scene->mRootNode, scene);
}

Model::~Model()
{
    for (auto texture : mTexturesLoaded)
    {
        glDeleteTextures(1, &texture.id);
    }
}

void Model::Draw(GLuint shaderId)
{
    glUseProgram(shaderId);
    for (size_t i = 0; i < mMeshes.size(); i++)
        mMeshes[i].Draw(shaderId);
}

void Model::ProcessNodeRecursive(aiNode* node, const aiScene* scene)
{
    for (size_t i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        mMeshes.emplace_back(ProcessMesh(mesh, scene));
    }

    for (size_t i = 0; i < node->mNumChildren; i++)
        ProcessNodeRecursive(node->mChildren[i], scene);
}

Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;

    for (size_t i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 v;
        v.x = mesh->mVertices[i].x;
        v.y = mesh->mVertices[i].y;
        v.z = mesh->mVertices[i].z;
        vertex.position = v;

        v.x = mesh->mNormals[i].x;
        v.y = mesh->mNormals[i].y;
        v.z = mesh->mNormals[i].z;
        vertex.normal = v;

        if (mesh->mTextureCoords[0])
        {
            glm::vec2 texV;
            texV.x = mesh->mTextureCoords[0][i].x;
            texV.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoords = texV;
        }
        else
            vertex.texCoords = glm::vec2(0.0f, 0.0f);

        vertices.emplace_back(vertex);
    }

    for (size_t i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (size_t j = 0; j < face.mNumIndices; j++)
            indices.emplace_back(face.mIndices[j]);
    }

    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        std::vector<Texture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        std::vector<Texture> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }

    return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
    std::vector<Texture> textures;

    for (size_t i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, static_cast<GLuint>(i), &str);
        bool skip = false;

        std::string path = str.C_Str();
        auto search = mLoadedTextures.find(path);
        if (search != mLoadedTextures.end())
        {
            textures.emplace_back(search->second);
            skip = true;
            break;
        }

        if (!skip)
        {
            Texture texture;
            GLuint texId;
            helper::loadTexture(fmt::format("resources/{}", str.C_Str()).c_str(), texId);
            texture.id = texId;
            texture.type = typeName;
            texture.path = str.C_Str();

            textures.emplace_back(texture);
            // mTexturesLoaded.emplace_back(texture);
            mLoadedTextures.emplace(std::make_pair(path, texture));
        }
    }

    return textures;
}