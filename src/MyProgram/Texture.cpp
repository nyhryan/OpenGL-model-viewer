#include "Texture.h"

#include <gl/gl3w.h>

#include <stb_image.h>
#include <fmt/core.h>

Texture::Texture(const std::string& path)
{
    // load image file from path
    int w, h, ch;
    unsigned char* imgData = stbi_load(path.c_str(), &w, &h, &ch, 0);
    if (!imgData)
    {
        fmt::print(stderr, "[TEXTURE-ERROR] Failed to load \"{}\" file\n", path);
        stbi_image_free(imgData);
        return;
    }

    fmt::print("[TEXTURE] Successfully loaded \"{}\"\n", path);

    // create OpenGL texture
    glGenTextures(1, &mId);
    glBindTexture(GL_TEXTURE_2D, mId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum pixelFormat = ch == 3 ? GL_RGB : GL_RGBA;
    if (imgData)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, pixelFormat, GL_UNSIGNED_BYTE, imgData);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(imgData);
}

Texture::~Texture()
{
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &mId);
}

Texture::Texture(const Texture&)
{

}
