#pragma once

#include <glad/glad.h>

namespace Hd {

class VBO {
public:
    VBO(GLint target);
    ~VBO();

    void Bind();
    void Data(const void* data, size_t size);

private:

    GLuint mId;
    GLint mTarget;

    VBO(const VBO&) = delete;
    VBO& operator=(const VBO&) = delete;
};

inline VBO::VBO(GLint target) : mTarget(target) {
    glGenBuffers(1, &mId);
}

inline VBO::~VBO() {
    glBindBuffer(mTarget, 0);
    glDeleteBuffers(1, &mId);
}

inline void VBO::Bind() {
    glBindBuffer(mTarget, mId);
}

inline void VBO::Data(const void* data, size_t size) {
    Bind();
    glBufferData(mTarget, size, data, GL_STATIC_DRAW);
}

}
