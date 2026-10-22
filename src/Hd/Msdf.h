#pragma once

// multi-channel signed distance fields of glyphs with msdfgen. no opengl, so
// tests can check the fields.

#include <glm/glm.hpp>
#include <stb/stb_truetype.h>

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace Hd::msdf {

// the field of one glyph, rows bottom up. the median of the three channels
// is the distance, 128 on the outline.
struct Field {
    // 0: nothing to draw, e.g. a space
    int w = 0, h = 0;
    // bottom left corner relative to the pen on the baseline, pixels, y up
    glm::vec2 origin { 0.0f };
    std::vector<unsigned char> rgb;
};

// scale: pixels per font unit. distances reach range / 2 pixels on each side
// of the outline. inside and outside follow the nonzero rule, so glyphs with
// overlapping contours are filled.
Field generate(const stbtt_fontinfo& info, int glyph, float scale, float range);

// fields of all codepoints the font has, packed into one square texture
struct Atlas {
    struct Glyph {
        // in the atlas, pixels. size 0: nothing to draw
        glm::ivec2 pos { 0 }, size { 0 };
        glm::vec2 origin { 0.0f };
    };

    int size = 0;
    // size * size * 3, rows bottom up
    std::vector<unsigned char> rgb;
    std::unordered_map<uint32_t, Glyph> glyphs;
};

// 1024 pixels wide, or 2048 or 4096 if they don't fit. glyphs that still
// don't fit are left out.
Atlas pack(const stbtt_fontinfo& info, const std::vector<uint32_t>& codepoints, float scale, float range);

}
