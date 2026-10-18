#pragma once

#include <Hd/Font.h>
#include <Hd/Shader.h>
#include <Hd/VAO.h>
#include <Hd/VBO.h>

#include <glm/glm.hpp>

#include <memory>
#include <string_view>
#include <vector>

namespace Hd {

// batched 2d drawing with anti aliased edges. every shape is a quad whose
// fragment shader evaluates the distance to the shape's outline, so lines of
// any width and direction, circles and rounded rects are smooth at any zoom.
// text uses a signed distance field font.
//
// shapes are collected between begin() and end() and drawn in one call, in
// the order they were added. sizes are in world units, a shape thinner than
// a pixel is drawn one pixel wide and fainter so it doesn't disappear.
class Renderer2D {
public:
    enum class Cap {
        round,
        butt,
    };

    enum class Align {
        left,
        center,
        right,
    };

    // font: a ttf file, the library's default font if null
    explicit Renderer2D(const unsigned char* font = nullptr);

    Renderer2D(const Renderer2D&) = delete;
    Renderer2D& operator=(const Renderer2D&) = delete;

    // vp maps world to clip space for the current viewport. blending is
    // enabled and left enabled.
    void begin(const glm::mat4& vp);
    // draws everything collected since begin()
    void end();

    // size of a pixel in world units, valid after begin()
    float pixel() const { return px; }

    void line(glm::vec2 a, glm::vec2 b, float width, glm::vec4 color, Cap cap = Cap::round);
    // quadratic bezier from a to b with control point c, as round capped lines
    void bezier(glm::vec2 a, glm::vec2 c, glm::vec2 b, float width, glm::vec4 color, int segments = 16);

    void circle(glm::vec2 center, float radius, glm::vec4 color);
    void ring(glm::vec2 center, float radius, float width, glm::vec4 color);

    void rect(glm::vec2 a, glm::vec2 b, glm::vec4 color, float corner = 0.0f);
    void rectOutline(glm::vec2 a, glm::vec2 b, float width, glm::vec4 color, float corner = 0.0f);

    // one line of utf-8 text. pos is on the baseline, height is from the
    // lowest descender to the highest ascender.
    void text(std::string_view s, glm::vec2 pos, float height, glm::vec4 color, Align align = Align::left);
    float textWidth(std::string_view s, float height) const { return m_font->width(s) * height; }

    const Font& font() const { return *m_font; }

private:
    struct Instance {
        glm::vec4 a; // points
        glm::vec4 b; // x: half width or radius, y: outline half width, z: corner, w: type
        glm::vec4 color;
        glm::vec4 uv;
    };

    void add(const Instance& i);

    std::unique_ptr<Font> m_font;
    Shader shader;
    VAO vao;
    VBO vbo;
    size_t capacity = 0;

    std::vector<Instance> batch;
    glm::mat4 vp { 1.0f };
    float px = 1.0f;
};

}
