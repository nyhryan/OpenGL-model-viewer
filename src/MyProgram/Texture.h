#pragma once

#include <gl/gl3w.h>

#ifdef STB_IMAGE_IMPLEMENTATION
#undef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include <stb_image.h>

#include <string>

class Texture
{
public:
    Texture(const std::string& path);
    ~Texture();
    Texture(const Texture&);

public:
    GLuint GetTextureId() const { return mId; }

private:
    GLuint mId;
};
