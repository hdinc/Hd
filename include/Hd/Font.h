#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stb/stb_truetype.h>

#include <cstdint>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Hd {

// glyphs of a truetype font in three forms, covering ascii, latin-1 and a
// few more letters (e.g. turkish), others are drawn as '?':
// - their outlines as curves in a float texture, for drawing large text
//   exactly
// - multi-channel signed distance fields in one texture, sharp corners at
//   any size but no strokes thinner than a texel
// - with freetype, hinted bitmaps for small text, made for each pixel size
//   when first asked for
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
        // the outline: curves in curveTexture(), their bounds relative to
        // the pen, units of the text height like the curves
        int first = 0, count = 0;
        glm::vec2 lo { 0.0f }, hi { 0.0f };
    };

    // a hinted glyph, pixels
    struct Bitmap {
        // in bitmapTexture(), bottom left. size 0: nothing to draw
        glm::ivec2 pos { 0 }, size { 0 };
        // bottom left relative to the pen on the baseline, y up
        glm::ivec2 bearing { 0 };
    };

    // ttf: the font file, the default font of the library if null. pixel
    // height of the glyphs in the texture, larger is more precise. index:
    // the font in a collection (.ttc). throws std::runtime_error if the
    // file is not a font stb_truetype can read.
    explicit Font(const unsigned char* ttf = nullptr, int pixelHeight = 48, int index = 0);
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
    // how far the distance fields reach, in texture pixels
    float range() const { return m_range; }
    // the outlines, two rgba32f texels per quadratic curve (p0 p1, p2), in
    // rows of curveRow texels
    GLuint curveTexture() const { return m_curveTexture; }
    static constexpr int curveRow = 1024;

    // hinted bitmaps need freetype
    bool hinting() const { return m_hinted != nullptr; }
    // the glyph of a text pixelHeight pixels high (ascent - descent), in the
    // same place as the distance field one. null without freetype, or when
    // the cache can't take more glyphs until the next batch.
    const Bitmap* bitmap(uint32_t codepoint, int pixelHeight);
    // the texture of the bitmaps with the glyphs added so far, 0 without
    // freetype
    GLuint bitmapTexture();
    // a new batch of drawing starts and nothing refers to bitmaps any more:
    // a full cache is emptied here
    void newBatch();

    // next codepoint of utf-8 text, advances i. invalid bytes are returned as is.
    static uint32_t decode(std::string_view s, size_t& i);

private:
    struct Hinted;

    std::vector<unsigned char> data;
    stbtt_fontinfo info;
    float scale;
    int px;

    std::unordered_map<uint32_t, Glyph> glyphs;
    Glyph fallback;

    float m_ascent;
    float m_descent;
    float m_capHeight;
    float m_range;

    GLuint m_texture = 0;
    GLuint m_curveTexture = 0;
    std::unique_ptr<Hinted> m_hinted;
};

}
