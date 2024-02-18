#include "GL/gl3w.h"

#include "helper.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <fmt/core.h>

#include <algorithm>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cmath>

static std::string readShader(const std::string& path)
{
    std::ifstream is(path, std::ios::binary);
    if (!is.is_open())
    {
        fmt::print(stderr, "Failed to open file: {}\n", path);
        return "";
    }

    is.seekg(0, std::ios::end);
    int length = static_cast<int>(is.tellg());

    is.seekg(0, std::ios::beg);

    std::stringstream ss;
    ss << is.rdbuf();
    is.close();

    return ss.str();
}

GLuint helper::loadShaders(helper::ShaderInfo* shaders)
{
    if (shaders == nullptr)
    {
        return 0;
    }

    GLuint program = glCreateProgram();

    helper::ShaderInfo* currentShader = shaders;
    while (currentShader->type != GL_NONE)
    {
        GLuint shader = glCreateShader(currentShader->type);
        currentShader->shader = shader;

        std::string source_string = readShader(currentShader->filename);
        if (source_string.empty())
        {
            for (currentShader = shaders; currentShader->type != GL_NONE; ++currentShader)
            {
                glDeleteShader(currentShader->shader);
                currentShader->shader = 0;
            }
            return 0;
        }

        const GLchar* source = source_string.c_str();
        glShaderSource(shader, 1, &source, nullptr);

        glCompileShader(shader);
        GLint compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

        if (compiled != GL_TRUE)
        {
            GLsizei len;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);

            GLchar* log = new GLchar[len + 1];
            glGetShaderInfoLog(shader, len, &len, log);
            std::cerr << "Shader compilation failed: " << log << std::endl;
            delete[] log;

            return 0;
        }

        glAttachShader(program, shader);

        ++currentShader;
    }

    glLinkProgram(program);
    for (currentShader = shaders; currentShader->type != GL_NONE; ++currentShader)
    {
        glDeleteShader(currentShader->shader);
        currentShader->shader = 0;
    }

    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE)
    {
        GLsizei len;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);

        GLchar* log = new GLchar[len + 1];
        glGetProgramInfoLog(program, len, &len, log);
        std::cerr << "Shader linking failed: " << log << std::endl;
        delete[] log;

        for (currentShader = shaders; currentShader->type != GL_NONE; ++currentShader)
        {
            glDeleteShader(currentShader->shader);
            currentShader->shader = 0;
        }

        return 0;
    }

    return program;
}

void helper::GLDebugMessageCallback(GLenum source, GLenum type, GLuint id,
    GLenum severity, GLsizei length,
    const GLchar* msg, const void* data)
{
    char* _source;
    char* _type;
    char* _severity;

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:
            _source = "API";
            break;

        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            _source = "WINDOW SYSTEM";
            break;

        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            _source = "SHADER COMPILER";
            break;

        case GL_DEBUG_SOURCE_THIRD_PARTY:
            _source = "THIRD PARTY";
            break;

        case GL_DEBUG_SOURCE_APPLICATION:
            _source = "APPLICATION";
            break;

        case GL_DEBUG_SOURCE_OTHER:
            _source = "UNKNOWN";
            break;

        default:
            _source = "UNKNOWN";
            break;
    }

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:
            _type = "ERROR";
            break;

        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            _type = "DEPRECATED BEHAVIOR";
            break;

        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            _type = "UDEFINED BEHAVIOR";
            break;

        case GL_DEBUG_TYPE_PORTABILITY:
            _type = "PORTABILITY";
            break;

        case GL_DEBUG_TYPE_PERFORMANCE:
            _type = "PERFORMANCE";
            break;

        case GL_DEBUG_TYPE_OTHER:
            _type = "OTHER";
            break;

        case GL_DEBUG_TYPE_MARKER:
            _type = "MARKER";
            break;

        default:
            _type = "UNKNOWN";
            break;
    }

    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:
            _severity = "HIGH";
            break;

        case GL_DEBUG_SEVERITY_MEDIUM:
            _severity = "MEDIUM";
            break;

        case GL_DEBUG_SEVERITY_LOW:
            _severity = "LOW";
            break;

        case GL_DEBUG_SEVERITY_NOTIFICATION:
            _severity = "NOTIFICATION";
            break;

        default:
            _severity = "UNKNOWN";
            break;
    }

    /*
        Buffer detailed info:
        Buffer object 2
        (bound to GL_ELEMENT_ARRAY_BUFFER_ARB, usage hint is GL_STREAM_DRAW)
        will use VIDEO memory as the source for buffer object operations.
    */
    if (id == 131185) return;

    printf("[ID: %d | %s | %s] raised from %s\n",
        id, _type, _severity, _source);
    printf(">> %s\n\n", msg);

#ifdef _DEBUG
    if (type == GL_DEBUG_TYPE_ERROR)
        __debugbreak();
#endif
}

void helper::GLClearError()
{
    while (glGetError() != GL_NO_ERROR)
        ;
}

bool helper::GLLogCall(const char* fnName, const char* fileName, int line)
{
    while (GLenum error = glGetError())
    {
        // std::cout << ">> " << fnName << " at " << fileName << ":" << line << std::endl;
        return false;
    }
    return true;
}

void helper::loadTexture(const char* path, GLuint& texture)
{
    // stbi_set_flip_vertically_on_load(true);
    int w, h, ch;
    unsigned char* data = stbi_load(path, &w, &h, &ch, 0);
    if (!data)
    {
        std::cerr << "Failed to load texture1 file." << std::endl;
        stbi_image_free(data);
        return;
    }

    fmt::print("[TEXTURE-INFO] Successfully loaded {}\n", path);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum pixelFormat = ch == 3 ? GL_RGB : GL_RGBA;
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, pixelFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(data);
}

void helper::freeTextureImageData(helper::TextureImageData& data)
{
    stbi_image_free(data.data);
    data.width = 0;
    data.height = 0;
    data.channels = 0;
    data.data = nullptr;

}

int helper::getMipmapLevels(int w, int h)
{
    float bigger = static_cast<float>(w > h ? w : h);
    return static_cast<int>(std::log2f(bigger)) + 1;
}