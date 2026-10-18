#include <Hd/Renderer2D.h>
#include <Hd/Resource.h>

#include <glm/gtc/type_ptr.hpp>

#include <cstddef>

namespace Hd {

namespace {

// must match the shaders
enum type : int {
    capsule = 0,
    segment = 1,
    disc = 2,
    box = 3,
    glyph = 4,
};

}

Renderer2D::Renderer2D(const unsigned char* font)
    : m_font(std::make_unique<Font>(font))
    , shader(Resource::getLibRes("res/renderer2D/shape_vert.glsl").first, Resource::getLibRes("res/renderer2D/shape_frag.glsl").first)
{
    vao.Bind();
    vbo.Bind(GL_ARRAY_BUFFER);
    for (int k = 0; k < 4; k++) {
        glEnableVertexAttribArray(k);
        glVertexAttribPointer(k, 4, GL_FLOAT, GL_FALSE, sizeof(Instance), (void*)(k * sizeof(glm::vec4)));
        glVertexAttribDivisor(k, 1);
    }
    glBindVertexArray(0);
}

void Renderer2D::begin(const glm::mat4& m)
{
    vp = m;
    batch.clear();

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    // clip space is 2 wide, a world unit is |vp x axis| of it
    float scale = glm::length(glm::vec2(vp[0])) * viewport[2] / 2.0f;
    px = scale > 0 ? 1.0f / scale : 1.0f;
}

void Renderer2D::end()
{
    if (batch.empty()) return;

    glEnable(GL_BLEND);
    // keep the destination opaque, it may be shown as a texture
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    vbo.Bind(GL_ARRAY_BUFFER);
    if (batch.size() > capacity) {
        capacity = batch.size() * 2;
        glBufferData(GL_ARRAY_BUFFER, capacity * sizeof(Instance), nullptr, GL_STREAM_DRAW);
    }
    glBufferSubData(GL_ARRAY_BUFFER, 0, batch.size() * sizeof(Instance), batch.data());

    shader.Bind();
    glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(vp));
    glUniform1f(1, px);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_font->texture());
    glUniform1i(2, 0);

    vao.Bind();
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, (GLsizei)batch.size());
    glBindVertexArray(0);

    batch.clear();
}

void Renderer2D::add(const Instance& i)
{
    batch.push_back(i);
}

void Renderer2D::line(glm::vec2 a, glm::vec2 b, float width, glm::vec4 color, Cap cap)
{
    add({ { a, b }, { width / 2, 0, 0, cap == Cap::round ? capsule : segment }, color, {} });
}

void Renderer2D::bezier(glm::vec2 a, glm::vec2 c, glm::vec2 b, float width, glm::vec4 color, int segments)
{
    glm::vec2 prev = a;
    for (int k = 1; k <= segments; k++) {
        float t = float(k) / segments;
        glm::vec2 p = (1 - t) * (1 - t) * a + 2 * (1 - t) * t * c + t * t * b;
        line(prev, p, width, color);
        prev = p;
    }
}

void Renderer2D::circle(glm::vec2 center, float radius, glm::vec4 color)
{
    add({ { center, center }, { radius, 0, 0, disc }, color, {} });
}

void Renderer2D::ring(glm::vec2 center, float radius, float width, glm::vec4 color)
{
    add({ { center, center }, { radius, width / 2, 0, disc }, color, {} });
}

void Renderer2D::rect(glm::vec2 a, glm::vec2 b, glm::vec4 color, float corner)
{
    add({ { glm::min(a, b), glm::max(a, b) }, { 0, 0, corner, box }, color, {} });
}

void Renderer2D::rectOutline(glm::vec2 a, glm::vec2 b, float width, glm::vec4 color, float corner)
{
    add({ { glm::min(a, b), glm::max(a, b) }, { 0, width / 2, corner, box }, color, {} });
}

void Renderer2D::text(std::string_view s, glm::vec2 pos, float height, glm::vec4 color, Align align)
{
    if (align != Align::left) pos.x -= textWidth(s, height) * (align == Align::center ? 0.5f : 1.0f);

    uint32_t prev = 0;
    for (size_t i = 0; i < s.size();) {
        uint32_t cp = Font::decode(s, i);
        if (prev) pos.x += m_font->kerning(prev, cp) * height;
        auto& g = m_font->glyph(cp);
        if (g.max.x > g.min.x) add({ { pos + g.min * height, pos + g.max * height }, { 0, 0, 0, glyph }, color, { g.uv0, g.uv1 } });
        pos.x += g.advance * height;
        prev = cp;
    }
}

}
