#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stb/stb_truetype.h>

#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Hd {

// glyphs of a truetype font as signed distance fields in one texture, so text
// stays sharp at any size. covers ascii, latin-1 and a few more letters
// (e.g. turkish), others are drawn as '?'.
//
// sizes are in units of the text height (ascent - descent): a glyph of a
// text drawn with height h is h times as large.
class Font {
public:
    struct Glyph {
        // texture coordinates
        glm::vec2 uv0, uv1;
        // quad relative to the pen on the baseline, y up
        glm::vec2 min, max;
        float advance;
    };

    // ttf: the font file, the default font of the library if null. pixel
    // height of the glyphs in the texture, larger is more precise.
    explicit Font(const unsigned char* ttf = nullptr, int pixelHeight = 48);
    ~Font();

    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;

    const Glyph& glyph(uint32_t codepoint) const;
    float kerning(uint32_t a, uint32_t b) const;

    // width of a single line of text with height 1
    float width(std::string_view utf8) const;

    float ascent() const { return m_ascent; }
    float descent() const { return m_descent; }
    // height of capital letters and digits above the baseline, for centering
    float capHeight() const { return m_capHeight; }

    GLuint texture() const { return m_texture; }

    // next codepoint of utf-8 text, advances i. invalid bytes are returned as is.
    static uint32_t decode(std::string_view s, size_t& i);

private:
    std::vector<unsigned char> data;
    stbtt_fontinfo info;
    float scale;
    int px;

    std::unordered_map<uint32_t, Glyph> glyphs;
    Glyph fallback;

    float m_ascent;
    float m_descent;
    float m_capHeight;

    GLuint m_texture = 0;
};

}
