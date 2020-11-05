#include <Hd/FrameBuffer.h>

namespace Hd {

FrameBuffer::FrameBuffer(Texture& tex)
    : m_Texture(tex)
{
    glGenFramebuffers(1, &m_FrameBufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferId);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex.Id(), 0);
}

}
