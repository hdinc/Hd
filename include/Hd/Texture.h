#pragma once

#include <glad/glad.h>

namespace Hd {

class Texture {
public:
    Texture(GLuint id, int w, int h);
    Texture(Texture&& other);
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    void Bind();

    GLuint Id();
    int GetWidth();
    int GetHeight();

public:
    static Texture fromFile(const char* fname);
    static Texture fromBitMap(const void* buf, int w, int h, int nc, GLenum format);
    static Texture fromMemory(const void* buf, int size);
    static Texture emptyTexture(int w, int h);

private:
    GLuint mTextureId;
    int mWidth;
    int mHeight;
};

inline void Texture::Bind()
{
    glBindTexture(GL_TEXTURE_2D, mTextureId);
}

inline Texture::~Texture()
{
    if (mTextureId) glDeleteTextures(1, &mTextureId);
}

inline int Texture::GetWidth() { return mWidth; }
inline int Texture::GetHeight() { return mHeight; }
inline GLuint Texture::Id() { return mTextureId; }

inline Texture::Texture(GLuint id, int w, int h)
    : mTextureId(id)
    , mWidth(w)
    , mHeight(h)
{
}

inline Texture::Texture(Texture&& other)
{
    mTextureId = other.mTextureId;
    other.mTextureId = 0;
    mWidth = other.mWidth;
    mHeight = other.mHeight;
}

}
