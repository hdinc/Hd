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
// the order they were added. filled polygons are drawn before them, under
// all lines and text. sizes are in world units, a shape thinner than
// a pixel is drawn one pixel wide and fainter so it doesn't disappear.
//
// text of hintBelow pixels and more is drawn exactly from the font's
// outlines. smaller text uses hinted bitmaps at its pixel size, on whole
// pixels, when the font has them (freetype) and the view isn't rotated,
// else the font's distance fields.
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

    enum class Grid {
        dots,
        lines,
        checkers,
    };

    // font: a ttf file, the library's default font if null
    explicit Renderer2D(const unsigned char* font = nullptr);

    Renderer2D(const Renderer2D&) = delete;
    Renderer2D& operator=(const Renderer2D&) = delete;

    // vp maps world to clip space for the current viewport. blending is
    // enabled and left enabled.
    void begin(const glm::mat4& vp);
    // draws everything collected since begin(), the fills first
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

    // a filled polygon (any winding, may be concave), drawn under the other
    // shapes at end(). not anti aliased, outline it for smooth edges.
    void fill(const std::vector<glm::vec2>& polygon, glm::vec4 color);
    // a filled triangle, like fill
    void triangle(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec4 color);
    // triangles of a simple polygon by ear clipping, three points each, any
    // winding. a polygon that isn't simple keeps the triangles found until
    // no ear is left. for caching the triangles of a polygon drawn often.
    static std::vector<glm::vec2> triangulate(std::vector<glm::vec2> polygon);

    // marks every spacing world units, bigger ones every 5 * spacing, over
    // the whole viewport, drawn at once (call it after begin() and before
    // what goes over it). fine: how much the small marks show, 0 to 1, e.g.
    // fading them out while zooming out. the marks are anti aliased and
    // scale framebuffer pixels big (more than 1 on hi-dpi screens).
    void grid(Grid style, float spacing, float fine, glm::vec4 color, float scale = 1.0f);

    // one line of utf-8 text. pos is on the baseline, height is from the
    // lowest descender to the highest ascender.
    void text(std::string_view s, glm::vec2 pos, float height, glm::vec4 color, Align align = Align::left);
    float textWidth(std::string_view s, float height) const { return m_font->width(s) * height; }

    const Font& font() const { return *m_font; }
    // hinted bitmaps for small text, see the class comment
    bool hinting = true;
    int hintBelow = 24;
    // replaces the font, see Font. keeps the old one if this one throws.
    void setFont(const unsigned char* ttf, int index = 0);

private:
    struct Instance {
        glm::vec4 a; // points
        glm::vec4 b; // x: half width or radius, y: outline half width, z: corner, w: type
        glm::vec4 color;
        glm::vec4 uv;
    };

    struct FillVertex {
        glm::vec2 pos;
        glm::vec4 color;
    };

    void add(const Instance& i);
    void drawFills();
    // between world and pixels of the viewport, y up
    glm::vec2 toPixels(glm::vec2 world) const;
    glm::vec2 fromPixels(glm::vec2 pixels) const;

    std::unique_ptr<Font> m_font;
    Shader shader;
    VAO vao;
    VBO vbo;
    size_t capacity = 0;

    std::vector<Instance> batch;

    Shader fillShader;
    VAO fillVao;
    VBO fillVbo;
    size_t fillCapacity = 0;
    std::vector<FillVertex> fills;

    Shader gridShader;
    // no attributes, the shader makes the quad
    VAO gridVao;

    glm::mat4 vp { 1.0f };
    glm::mat4 inv { 1.0f };
    glm::vec2 viewport { 1.0f };
    // the view isn't rotated, text can sit on whole pixels
    bool snap = false;
    float px = 1.0f;
};

}
