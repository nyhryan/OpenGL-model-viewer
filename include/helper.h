#pragma once

#include "GL/gl3w.h"

#include <string>

#define ASSERT(x) \
    if (!(x)) { __debugbreak(); }

#define GLCall(x)           \
    helper::GLClearError(); \
    x;                      \
    ASSERT(helper::GLLogCall(#x, __FILE__, __LINE__))


namespace helper
{
    struct ShaderInfo
    {
        GLenum type;
        std::string filename;
        GLuint shader;
    };

    struct TextureImageData
    {
        int width = 0;
        int height = 0;
        int channels = 0;
        unsigned char* data = nullptr;
    };


    GLuint loadShaders(ShaderInfo*);

    void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id,
        GLenum severity, GLsizei length,
        const GLchar* msg, const void* data);

    void GLClearError();
    bool GLLogCall(const char* fnName, const char* fileName, int line);

    void loadTexture(const char* path, GLuint& texture);
    void freeTextureImageData(TextureImageData& data);
    int getMipmapLevels(int w, int h);
}
