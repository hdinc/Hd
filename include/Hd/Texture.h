#include <glad/glad.h>

namespace Hd {

class Texture {
public:
    Texture(const char* filename);
    ~Texture();
    GLuint Id() { return m_TextureId; }
    void Bind(unsigned int n);

private:
    GLuint m_TextureId;
};

inline void Texture::Bind(unsigned int n)
{
    glActiveTexture(GL_TEXTURE0 + n);
    glBindTexture(GL_TEXTURE_2D, m_TextureId);
}

inline Texture::~Texture() { glDeleteTextures(1, &m_TextureId); }

}
