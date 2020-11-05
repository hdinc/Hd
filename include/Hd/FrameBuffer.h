#include <glad/glad.h>
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
    Texture& texture;
};

inline void FrameBuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferId);
    glViewport(0, 0, texture.GetWidth(), texture.GetHeight());
}

inline GLuint FrameBuffer::Id() { return m_FrameBufferId; }
