#include <stb/stb_image.h>
#include <glad/glad.h>
#include <Hd/Texture.h>

namespace Hd {
Texture::Texture(const char* filename)
{

    glGenTextures(1, &m_TextureId);
    glBindTexture(GL_TEXTURE_2D, m_TextureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    void* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (!data) {
        puts("Failed to load texture");
        exit(1);
    }
    // TODO: Alpha channel
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
}

}
