#pragma once

#include <glad/glad.h>
#include <utility>

namespace Hd {

class VBO {
public:
    VBO();
    ~VBO();

    VBO(const VBO&) = delete;
    VBO& operator=(const VBO&) = delete;

    VBO(VBO&& other) noexcept;
    VBO& operator=(VBO&& other) noexcept;

    void Bind(GLenum target);

private:
    GLuint m_id = 0;
};

inline VBO::VBO()
{
    glGenBuffers(1, &m_id);
}

inline VBO::~VBO()
{
    if (m_id)
        glDeleteBuffers(1, &m_id);
}

inline void VBO::Bind(GLenum target)
{
    glBindBuffer(target, m_id);
}

inline VBO::VBO(VBO&& other) noexcept
    : m_id(std::exchange(other.m_id, 0))
{
}

inline VBO& VBO::operator=(VBO&& other) noexcept
{
    std::swap(m_id, other.m_id);
    return *this;
}

}
