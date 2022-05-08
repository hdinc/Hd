#pragma once

#include <Hd/Texture.h>

namespace Hd {

class FrameBuffer {
public:
    FrameBuffer(Hd::Texture& tex);
    ~FrameBuffer();
    void Bind();
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

inline GLuint FrameBuffer::Id() { return m_FrameBufferId; }

inline FrameBuffer::~FrameBuffer()
{
    glDeleteFramebuffers(1, &m_FrameBufferId);
}

}
