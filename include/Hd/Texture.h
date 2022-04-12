#pragma once

#include <glad/glad.h>

namespace Hd {

class Texture {
public:
    Texture(const char* file, long size);
    ~Texture();
    GLuint Id() { return m_TextureId; }
    void Bind(unsigned int n);
    int GetWidth();
    int GetHeight();

private:
    GLuint m_TextureId;
    int m_Width;
    int m_Height;
};

inline void Texture::Bind(unsigned int n)
{
    glActiveTexture(GL_TEXTURE0 + n);
    glBindTexture(GL_TEXTURE_2D, m_TextureId);
}

inline Texture::~Texture() { glDeleteTextures(1, &m_TextureId); }

inline int Texture::GetWidth() { return m_Width; }

inline int Texture::GetHeight() { return m_Height; }

}
