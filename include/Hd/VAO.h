#pragma once

#include <glad/glad.h>
#include <utility>

namespace Hd {

class VAO {
public:
    VAO();
    ~VAO();

    VAO(const VAO&) = delete;
    VAO& operator=(const VAO&) = delete;

    VAO(VAO&& other) noexcept;
    VAO& operator=(VAO&& other) noexcept;

    void Bind();

private:
    GLuint m_id = 0;
};

inline VAO::VAO()
{
    glGenVertexArrays(1, &m_id);
}

inline VAO::~VAO()
{
    if (m_id) {
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &m_id);
    }
}

inline void VAO::Bind()
{
    glBindVertexArray(m_id);
}

inline VAO::VAO(VAO&& other) noexcept
    : m_id(std::exchange(other.m_id, 0))
{
}

inline VAO& VAO::operator=(VAO&& other) noexcept
{
    std::swap(m_id, other.m_id);
    return *this;
}

}
