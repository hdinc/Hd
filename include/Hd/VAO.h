#pragma once

#include <glad/glad.h>

namespace Hd {

class VAO {
public:
    VAO();
    ~VAO();

    void Bind();

private:

    GLuint mId;

    VAO(const VAO&) = delete;
    VAO& operator=(const VAO&) = delete;
};

inline VAO::VAO() {
    glGenVertexArrays(1, &mId);
}

inline VAO::~VAO() {
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &mId);
}

inline void VAO::Bind() {
    glBindVertexArray(mId);
}

}
