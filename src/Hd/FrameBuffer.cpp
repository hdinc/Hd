#include <Hd/FrameBuffer.h>

namespace Hd {

FrameBuffer::FrameBuffer(Texture& tex)
    : m_Texture(tex)
{
    glGenFramebuffers(1, &m_FrameBufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferId);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex.Id(), 0);
}

}
