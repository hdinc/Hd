#pragma once

// glyph outlines as quadratic curves for drawing them exactly, at any size.
// no opengl, so tests can check them.

#include <glm/glm.hpp>
#include <stb/stb_truetype.h>

#include <vector>

namespace Hd::outline {

// a quadratic bezier curve in font units. lines have their control point in
// the middle.
struct Curve {
    glm::vec2 p0, p1, p2;
};

// the outline of a glyph, each curve monotonic in x and y: a horizontal or
// vertical line crosses it at most once, and does if its end points are on
// both sides. cubic curves (cff fonts) are approximated.
std::vector<Curve> curves(const stbtt_fontinfo& info, int glyph);

// winding number of the curves around p, crossings of the ray to +x
int winding(const std::vector<Curve>& curves, glm::vec2 p);

}
