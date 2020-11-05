#include <Hd/FrameBuffer.h>

namespace Hd {

FrameBuffer::FrameBuffer(Texture& tex)
{
    glGenFramebuffers(1, &m_FrameBufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferID);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex.ID(), 0);
}
}
