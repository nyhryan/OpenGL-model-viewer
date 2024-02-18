#pragma once

#include <GL/gl3w.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <vector>
#include <unordered_map>

class Shader
{
public:
    Shader();
    ~Shader();

public:
    void AddShader(GLenum type, const std::string& path);
    void Link();
    void Use() { glUseProgram(mShaderId); }
    void Recompile();

public:
    GLuint GetId() { return mShaderId; }
    void SetInt(const char* uniformName, int data);
    void SetFloat(const char* uniformName, float data);
    void SetFloat3(const char* uniformName, const float* data);
    void SetUniformVec3(const char* uniformName, const glm::vec3& data);
    void SetUniformMat3(const char* uniformName, const glm::mat3& data);
    void SetUniformMat4(const char* uniformName, const glm::mat4& data);

private:
    GLuint mShaderId;
    std::vector<GLuint> shaders;
    std::unordered_map<std::string, GLenum> shaderData;
};
