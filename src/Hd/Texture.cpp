#include <glad/glad.h>
#include <Hd/Texture.h>
#include <opencv2/opencv.hpp>
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

Texture Texture::fromFile(const char* fname) {

    auto image = cv::imread(fname, cv::IMREAD_UNCHANGED);

    if (image.empty()) {
        throw 1;
    }

    int nc = image.channels();

    GLenum f;

    if (nc == 3) {
        f = GL_BGR;
    } else if (nc == 4) {
        f = GL_BGRA;
    } else {
        throw 2;
    }

    GLuint id = create_texture(image.cols, image.rows, image.data, nc, f);

    return Hd::Texture{id, image.cols, image.rows};
}

Texture Texture::fromBitMap(const void* buf, int w, int h, int nc, GLenum format) {
    GLuint id = create_texture(w, h, buf, nc, format);
    return Hd::Texture{id, w, h};
}

Texture Texture::fromMemory(const void* buf, int size) {

    cv::_InputArray a(static_cast<const char*>(buf), size);

    auto image = cv::imdecode(a, cv::IMREAD_UNCHANGED);

    if (image.empty()) {
        throw 1;
    }

    int nc = image.channels();

    GLenum f;

    if (nc == 3) {
        f = GL_BGR;
    } else if (nc == 4) {
        f = GL_BGRA;
    } else {
        throw 2;
    }

    GLuint id = create_texture(image.cols, image.rows, image.data, nc, f);

    return Hd::Texture{id, image.cols, image.rows};

}

Texture Texture::emptyTexture(int w, int h) {
    GLuint id = create_texture(w, h, 0, 4, GL_RGBA);
    return Hd::Texture{id, w, h};
}

}
