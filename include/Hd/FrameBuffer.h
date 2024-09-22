#pragma once

#include <glad/glad.h>
#include <Hd/Texture.h>

namespace Hd {

class FrameBuffer {
public:
    FrameBuffer(Hd::Texture& tex);
    ~FrameBuffer();

    FrameBuffer(const FrameBuffer&) = delete;
    FrameBuffer& operator=(const FrameBuffer&) = delete;

    void Bind();
    void Bind(int w, int h);
    GLuint Id();

private:
    GLuint m_FrameBufferId;
    Texture& m_Texture;
};

inline void FrameBuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferId);
    glViewport(0, 0, m_Texture.GetWidth(), m_Texture.GetHeight());
}

inline void FrameBuffer::Bind(int w, int h) {
    glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferId);
    glViewport(0, 0, w, h);
}

inline GLuint FrameBuffer::Id() { return m_FrameBufferId; }

inline FrameBuffer::~FrameBuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &m_FrameBufferId);
}

}
