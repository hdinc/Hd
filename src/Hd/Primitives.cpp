#include <Hd/Primitives.h>
#include <Hd/Resource.h>

#include <glm/gtc/type_ptr.hpp>

namespace Hd {

using Hd::Resource::getLibRes;

Primitives::Primitives()
    : lineShader(getLibRes("res/primitives/line_vertex.glsl").first, getLibRes("res/primitives/line_frag.glsl").first)
    , rectShader(getLibRes("res/primitives/rect_vertex.glsl").first, getLibRes("res/primitives/rect_frag.glsl").first)
    , rectTShader(getLibRes("res/primitives/rectT_vertex.glsl").first, getLibRes("res/primitives/rectT_texture.glsl").first)
{
    glGenVertexArrays(1, &vao);
}

Primitives::~Primitives() {
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao);
}

void Primitives::drawLine(Hd::Camera& c, glm::vec2 p1, glm::vec2 p2) {

    glm::vec2 p[2] = {p1, p2};

    lineShader.Bind();

    glUniformMatrix4fv(0, 1, false, glm::value_ptr(*c.getVP()));
    glUniform2fv(1, 2, &p[0].x);

    glBindVertexArray(vao);
    glDrawArrays(GL_LINES, 0, 4);

}

void Primitives::drawRect(Hd::Camera& c, glm::vec2 p1, glm::vec2 p2) {

    glm::vec2 p[4] = {
        {p1.x, p1.y},
        {p2.x, p1.y},
        {p2.x, p2.y},
        {p1.x, p2.y}
    };

    rectShader.Bind();

    glUniformMatrix4fv(0, 1, false, glm::value_ptr(*c.getVP()));
    glUniform2fv(1, 4, &p[0].x);

    glBindVertexArray(vao);
    glDrawArrays(GL_LINE_LOOP, 0, 4);

}

void Primitives::drawTexturedRect(Hd::Camera& c, glm::vec2 p1, glm::vec2 p2, Hd::Texture& t) {

    glm::vec2 p[4] = {
        {p1.x, p1.y},
        {p2.x, p1.y},
        {p1.x, p2.y},
        {p2.x, p2.y}
    };

    rectTShader.Bind();
    t.Bind();

    glUniformMatrix4fv(0, 1, false, glm::value_ptr(*c.getVP()));
    glUniform2fv(1, 4, &p[0].x);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

}

}
