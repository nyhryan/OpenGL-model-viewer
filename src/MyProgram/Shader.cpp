#include "Shader.h"

#include <GL/gl3w.h>

#include <fmt/core.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <unordered_map>

#include "helper.h"

Shader::Shader()
{
    mShaderId = glCreateProgram();
}

Shader::~Shader()
{
    glDeleteProgram(mShaderId);
}

void Shader::AddShader(GLenum type, const std::string& path)
{
    GLuint shader = glCreateShader(type);

    std::string str;
    {
        std::ifstream is(path, std::ios::binary);
        if (!is.is_open())
        {
            // std::cerr << "[SHADER-ERROR] Failed to open file: " << path << std::endl;
            fmt::print(stderr, "[SHADER-ERROR] Failed to open file: {}\n", path);
            return;
        }

        is.seekg(0, std::ios::end);
        int length = static_cast<int>(is.tellg());

#ifndef NDEBUG
        std::cout << "[SHADER-INFO] Read \'" << path << "\' (" << length << " Bytes)" << std::endl;
#endif

        is.seekg(0, std::ios::beg);
        std::stringstream ss;
        ss << is.rdbuf();
        is.close();

        str = ss.str();
    }

    shaderData.emplace(std::make_pair(path, type));

    size_t len = str.size();
    GLchar* str_c = new GLchar[len + 1];
    std::memcpy(str_c, str.c_str(), len);
    str_c[len] = '\0';

    glShaderSource(shader, 1, &str_c, nullptr);
    glCompileShader(shader);

    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (compiled != GL_TRUE)
    {
        GLsizei len;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);

        GLchar* log = new GLchar[len + 1];
        glGetShaderInfoLog(shader, len, &len, log);
        fmt::print(stderr, "[SHADER-ERROR] Shader compilation failed: {}\n", log);
        delete[] log;

        return;
    }

    glAttachShader(mShaderId, shader);
    shaders.push_back(shader);

    delete[] str_c;
}

void Shader::Link()
{
    glLinkProgram(mShaderId);

    for (auto it = shaders.begin(); it != shaders.end();)
    {
        glDeleteShader(*it);
        it = shaders.erase(it);
    }

    ASSERT(shaders.size() == 0);

    GLint linked;
    glGetProgramiv(mShaderId, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE)
    {
        GLsizei len;
        glGetProgramiv(mShaderId, GL_INFO_LOG_LENGTH, &len);

        GLchar* log = new GLchar[len + 1];
        glGetProgramInfoLog(mShaderId, len, &len, log);
        fmt::print(stderr, "[SHADER-ERROR] Shader linking failed: {}\n", log);
        delete[] log;
    }
}

void Shader::Recompile()
{
    glDeleteProgram(mShaderId);

    mShaderId = glCreateProgram();
    for (auto& data : shaderData)
    {
        AddShader(data.second, data.first);
    }
    Link();
}

void Shader::SetInt(const char* uniformName, int data)
{
    Use();
    glUniform1i(
        (glGetUniformLocation(mShaderId, uniformName)),
        data);
}

void Shader::SetFloat(const char* uniformName, float data)
{
    Use();
    glUniform1f(
        (glGetUniformLocation(mShaderId, uniformName)),
        data);
}

void Shader::SetFloat3(const char* uniformName, const float* data)
{
    Use();
    glUniform3fv(
        (glGetUniformLocation(mShaderId, uniformName)),
        1,
        data);
}

void Shader::SetUniformVec3(const char* uniformName, const glm::vec3& data)
{
    Use();
    glUniform3fv(
        (glGetUniformLocation(mShaderId, uniformName)),
        1,
        glm::value_ptr(data)  //
    );
}

void Shader::SetUniformMat3(const char* uniformName, const glm::mat3& data)
{
    Use();
    glUniformMatrix3fv(
        (glGetUniformLocation(mShaderId, uniformName)),
        1,
        GL_FALSE,
        glm::value_ptr(data)  //
    );
}

void Shader::SetUniformMat4(const char* uniformName, const glm::mat4& data)
{
    Use();
    glUniformMatrix4fv(
        (glGetUniformLocation(mShaderId, uniformName)),
        1,
        GL_FALSE,
        glm::value_ptr(data)  //
    );
}
