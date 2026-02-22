#pragma once

#include <glad/glad.h>

namespace Hd {

class SSBO {
public:
    SSBO();
    ~SSBO();

    void Bind();
    void BindBase(GLuint i);
    void Data(const void* data, size_t size, GLenum usage = GL_STATIC_DRAW);
    void SubData(const void* data, GLintptr offset, size_t size);

private:

    GLuint mId;

    SSBO(const SSBO&) = delete;
    SSBO& operator=(const SSBO&) = delete;
};

inline SSBO::SSBO() {
    glGenBuffers(1, &mId);
}

inline SSBO::~SSBO() {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glDeleteBuffers(1, &mId);
}

inline void SSBO::Bind() {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mId);
}

inline void SSBO::BindBase(GLuint i) {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i, mId);
}

inline void SSBO::Data(const void* data, size_t size, GLenum usage) {
    Bind();
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, usage);
}

inline void SSBO::SubData(const void* data, GLintptr offset, size_t size) {
    Bind();
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
}

}
