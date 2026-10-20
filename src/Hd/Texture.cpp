#include <glad/glad.h>
#include <Hd/Texture.h>
#include <stb/stb_image.h>
#include <stdio.h>

namespace Hd {

static GLuint create_texture(
    int w,
    int h,
    const void* b,
    int c,
    GLenum f,
    GLenum wrap_s = GL_REPEAT,
    GLenum wrap_t = GL_REPEAT,
    GLenum min_filter = GL_LINEAR_MIPMAP_LINEAR,
    GLenum mag_filter = GL_LINEAR
) {

    GLuint id;

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

    if (c == 3) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, f, GL_UNSIGNED_BYTE, b);
    } else if (c == 4) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, f, GL_UNSIGNED_BYTE, b);
    }

    glGenerateMipmap(GL_TEXTURE_2D);

    return id;
}

// pixels from stb_image as a texture: rgb or rgba, other images converted
// to rgba. stb keeps the first row on top like the textures were before.
static Texture from_stbi(stbi_uc* pixels, int w, int h, int nc) {

    if (!pixels) {
        throw 1;
    }

    GLuint id = create_texture(w, h, pixels, nc, nc == 3 ? GL_RGB : GL_RGBA);
    stbi_image_free(pixels);

    return Hd::Texture{id, w, h};
}

Texture Texture::fromFile(const char* fname) {

    int w, h, nc;
    if (!stbi_info(fname, &w, &h, &nc)) {
        throw 1;
    }
    int want = nc == 3 ? 3 : 4;
    auto pixels = stbi_load(fname, &w, &h, &nc, want);
    return from_stbi(pixels, w, h, want);
}

Texture Texture::fromBitMap(const void* buf, int w, int h, int nc, GLenum format) {
    GLuint id = create_texture(w, h, buf, nc, format);
    return Hd::Texture{id, w, h};
}

Texture Texture::fromMemory(const void* buf, int size) {

    auto data = static_cast<const stbi_uc*>(buf);
    int w, h, nc;
    if (!stbi_info_from_memory(data, size, &w, &h, &nc)) {
        throw 1;
    }
    int want = nc == 3 ? 3 : 4;
    auto pixels = stbi_load_from_memory(data, size, &w, &h, &nc, want);
    return from_stbi(pixels, w, h, want);
}

Texture Texture::emptyTexture(int w, int h) {
    GLuint id = create_texture(w, h, 0, 4, GL_RGBA);
    return Hd::Texture{id, w, h};
}

}
