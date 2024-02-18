#pragma once

#include <gl/gl3w.h>
#include <glm/glm.hpp>

#include <string>
#include <vector>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

struct Texture
{
    GLuint id;
    std::string type;
    std::string path;
};

class Mesh 
{
public:
    Mesh(
        const std::vector<Vertex>& vertices,
        const std::vector<GLuint>& indices,
        const std::vector<Texture>& textures
    );
    ~Mesh();

public:
    void Draw(GLuint shaderId);
    const GLuint getVAO() const { return VAO; }
    const GLuint getVBO() const { return VBO; }
    const GLuint getEBO() const { return EBO; }

private:
    void SetupMesh();

public:
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;

private:
    GLuint VAO, VBO, EBO;
};