#include "Hd.h"
#include <Hd/Graphics2D.h>
#include <Hd/Resource.h>

#include <glm/gtc/type_ptr.hpp>

namespace Hd {

using Hd::Resource::getLibRes;

Graphics2D::Graphics2D()
    : lineShader(getLibRes("res/graphics2D/line_vertex.glsl").first, getLibRes("res/graphics2D/line_frag.glsl").first)
    , rectShader(getLibRes("res/graphics2D/rect_vertex.glsl").first, getLibRes("res/graphics2D/rect_frag.glsl").first)
    , rectTShader(getLibRes("res/graphics2D/rectT_vertex.glsl").first, getLibRes("res/graphics2D/rectT_texture.glsl").first)
    , circleShader(getLibRes("res/graphics2D/circle_vert.glsl").first, getLibRes("res/graphics2D/circle_geo.glsl").first, getLibRes("res/graphics2D/circle_frag.glsl").first)
    , circleFilledShader(getLibRes("res/graphics2D/circle_vert.glsl").first, getLibRes("res/graphics2D/circle_filled_geo.glsl").first, getLibRes("res/graphics2D/circle_frag.glsl").first)
    , t(80)
{
    color = glm::vec4 { 1, 1, 1, 1 };
}

Graphics2D::~Graphics2D()
{
}

void Graphics2D::line(const glm::mat4& vp, glm::vec2 p1, glm::vec2 p2)
{
    glm::vec2 p[2] = { p1, p2 };

    lineShader.Bind();

    glUniform4fv(3, 1, &color.x);

    glUniformMatrix4fv(0, 1, false, glm::value_ptr(vp));
    glUniform2fv(1, 2, &p[0].x);

    vao.Bind();
    glDrawArrays(GL_LINES, 0, 2);
}

void Graphics2D::rect(const glm::mat4& vp, glm::vec2 p1, glm::vec2 p2, bool filled)
{
    glm::vec2 p[4];

    if (filled) {
        p[0] = { p1.x, p1.y };
        p[1] = { p2.x, p1.y };
        p[2] = { p1.x, p2.y };
        p[3] = { p2.x, p2.y };
    } else {
        p[0] = { p1.x, p1.y };
        p[1] = { p2.x, p1.y };
        p[2] = { p2.x, p2.y };
        p[3] = { p1.x, p2.y };
    }

    rectShader.Bind();

    glUniform4fv(5, 1, &color.x);

    glUniformMatrix4fv(0, 1, false, glm::value_ptr(vp));
    glUniform2fv(1, 4, &p[0].x);

    vao.Bind();
    if (filled) {
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    } else {
        glDrawArrays(GL_LINE_LOOP, 0, 4);
    }
}

void Graphics2D::texturedRect(const glm::mat4& vp, glm::vec2 p1, glm::vec2 p2, Hd::Texture& t)
{

    glm::vec2 p[4] = {
        { p1.x, p1.y },
        { p2.x, p1.y },
        { p1.x, p2.y },
        { p2.x, p2.y }
    };

    rectTShader.Bind();
    t.Bind();

    glUniformMatrix4fv(0, 1, false, glm::value_ptr(vp));
    glUniform2fv(1, 4, &p[0].x);

    vao.Bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Graphics2D::circle(const glm::mat4& vp, glm::vec2 c, float r, bool filled)
{
    if (filled) {
        circleFilledShader.Bind();
    } else {
        circleShader.Bind();
    }

    glUniformMatrix4fv(0, 1, false, glm::value_ptr(vp));
    glUniform2f(1, c.x, c.y);
    glUniform1f(2, r);
    glUniform4fv(3, 1, &color.x);

    vao.Bind();
    glDrawArrays(GL_POINTS, 0, 1);
}

void Graphics2D::setText(std::string text) {
    t.set_text(text);
}

void Graphics2D::text(const glm::mat4& vp, glm::vec2 p, float height) {
    t.draw(vp, p, height, color);
}

std::pair<glm::vec2, glm::vec2> Graphics2D::getTextBox() {
    return t.get_box();
}

void Graphics2D::setColor(glm::vec4 c)
{
    color = c;
}

}
