#include <Hd/Font.h>
#include <Hd/Resource.h>

#include "Msdf.h"
#include "Outline.h"

#ifdef HD_HAS_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

#include <climits>
#include <cmath>
#include <cstring>
#include <stdexcept>

namespace Hd {

namespace {

// distance range of the fields in pixels, both sides of the outline
constexpr float field_range = 6.0f;

std::vector<uint32_t> codepoints()
{
    std::vector<uint32_t> r;
    for (uint32_t c = 32; c < 127; c++)
        r.push_back(c);
    for (uint32_t c = 160; c < 256; c++)
        r.push_back(c);
    // turkish, greek letters common in electronics, arrows
    for (uint32_t c : { 0x11E, 0x11F, 0x130, 0x131, 0x15E, 0x15F, 0x3A9, 0x3BC, 0x394, 0x3C0, 0x2190, 0x2191, 0x2192, 0x2193, 0x2026 })
        r.push_back(c);
    return r;
}

}

#ifdef HD_HAS_FREETYPE

struct Font::Hinted {
    FT_Library lib = nullptr;
    FT_Face face = nullptr;
    // font units of the text height, the size stb_truetype scales by
    float unitsPerHeight = 1;
    // the pixel height set on the face
    int current = 0;

    // by pixel height << 32 | codepoint, nodes stay in place
    std::unordered_map<uint64_t, Bitmap> cache;

    // the glyphs in rows, bottom up like the texture
    int side = 1024;
    std::vector<unsigned char> pixels;
    int x = 1, y = 1, row = 0;
    bool full = false;
    // glyphs made in this batch, a new size costs some
    int budget = 0;

    GLuint texture = 0;
    bool resized = true;
    int dirtyLo = INT_MAX, dirtyHi = -1;

    ~Hinted()
    {
        if (texture) glDeleteTextures(1, &texture);
        if (face) FT_Done_Face(face);
        if (lib) FT_Done_FreeType(lib);
    }
};

#else

struct Font::Hinted { };

#endif

Font::Font(const unsigned char* ttf, int pixelHeight, int index)
    : px(pixelHeight)
{
    if (!ttf) ttf = (const unsigned char*)Resource::getLibRes("res/fonts/ElenaWebRegular.ttf").first;

    // the tables are read while drawing (kerning), keep a copy
    int offset = stbtt_GetFontOffsetForIndex(ttf, index);
    stbtt_fontinfo probe;
    if (offset < 0 || !stbtt_InitFont(&probe, ttf, offset)) throw std::runtime_error("not a truetype or opentype font");
    // the file size isn't known, the end of the last table is
    size_t size = 0;
    int tables = (ttf[offset + 4] << 8) | ttf[offset + 5];
    for (int t = 0; t < tables; t++) {
        auto e = ttf + offset + 12 + 16 * t;
        size_t start = ((size_t)e[8] << 24) | (e[9] << 16) | (e[10] << 8) | e[11];
        size_t len = ((size_t)e[12] << 24) | (e[13] << 16) | (e[14] << 8) | e[15];
        size = std::max(size, start + len);
    }
    data.assign(ttf, ttf + size);
    stbtt_InitFont(&info, data.data(), offset);

    scale = stbtt_ScaleForPixelHeight(&info, (float)px);
    int asc, desc, gap;
    stbtt_GetFontVMetrics(&info, &asc, &desc, &gap);
    m_ascent = asc * scale / px;
    m_descent = desc * scale / px;
    int x0, y0, x1, y1;
    m_capHeight = stbtt_GetCodepointBox(&info, 'H', &x0, &y0, &x1, &y1) ? y1 * scale / px : m_ascent * 0.7f;

    auto atlas = msdf::pack(info, codepoints(), scale, field_range);
    m_range = field_range;
    for (auto& [cp, a] : atlas.glyphs) {
        int adv, lsb;
        stbtt_GetCodepointHMetrics(&info, (int)cp, &adv, &lsb);

        Glyph g {};
        g.advance = adv * scale / px;
        if (a.size.x > 0) {
            // rows bottom up like the texture
            g.uv0 = glm::vec2(a.pos) / float(atlas.size);
            g.uv1 = glm::vec2(a.pos + a.size) / float(atlas.size);
            g.min = a.origin / float(px);
            g.max = (a.origin + glm::vec2(a.size)) / float(px);
        }
        glyphs[cp] = g;
    }

    // outlines, in units of the text height
    std::vector<glm::vec4> curves;
    float unit = scale / px;
    for (auto& [cp, g] : glyphs) {
        auto c = outline::curves(info, stbtt_FindGlyphIndex(&info, (int)cp));
        if (c.empty()) continue;
        g.first = (int)curves.size() / 2;
        g.count = (int)c.size();
        g.lo = glm::vec2(1e30f);
        g.hi = glm::vec2(-1e30f);
        for (auto& k : c) {
            glm::vec2 p0 = k.p0 * unit, p1 = k.p1 * unit, p2 = k.p2 * unit;
            curves.push_back({ p0, p1 });
            curves.push_back({ p2, 0, 0 });
            // monotonic curves stay inside their end points
            g.lo = glm::min(g.lo, glm::min(p0, p2));
            g.hi = glm::max(g.hi, glm::max(p0, p2));
        }
    }
    int rows = std::max(((int)curves.size() + curveRow - 1) / curveRow, 1);
    curves.resize((size_t)rows * curveRow, glm::vec4(0));
    glGenTextures(1, &m_curveTexture);
    glBindTexture(GL_TEXTURE_2D, m_curveTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, curveRow, rows, 0, GL_RGBA, GL_FLOAT, curves.data());
    // read with texelFetch only
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    fallback = glyphs.count('?') ? glyphs['?'] : Glyph {};

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, atlas.size, atlas.size, 0, GL_RGB, GL_UNSIGNED_BYTE, atlas.rgb.data());
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

#ifdef HD_HAS_FREETYPE
    auto h = std::make_unique<Hinted>();
    if (FT_Init_FreeType(&h->lib) == 0 && FT_New_Memory_Face(h->lib, data.data(), (FT_Long)data.size(), index, &h->face) == 0) {
        h->unitsPerHeight = float(asc - desc);
        h->pixels.assign((size_t)h->side * h->side, 0);
        m_hinted = std::move(h);
    }
#endif
}

Font::~Font()
{
    if (m_texture) glDeleteTextures(1, &m_texture);
    if (m_curveTexture) glDeleteTextures(1, &m_curveTexture);
}

const Font::Bitmap* Font::bitmap(uint32_t cp, int pixelHeight)
{
#ifdef HD_HAS_FREETYPE
    if (!m_hinted || pixelHeight <= 0) return nullptr;
    auto& h = *m_hinted;
    // the same glyph as the distance field, '?' for the missing ones
    if (!glyphs.count(cp)) cp = '?';
    uint64_t key = (uint64_t)pixelHeight << 32 | cp;
    if (auto it = h.cache.find(key); it != h.cache.end()) return &it->second;
    if (h.full || h.budget <= 0) return nullptr;
    h.budget--;

    if (h.current != pixelHeight) {
        // pixels per em for a text height of pixelHeight, like stb_truetype
        FT_F26Dot6 em = std::lround(pixelHeight * h.face->units_per_EM / h.unitsPerHeight * 64);
        if (FT_Set_Char_Size(h.face, 0, em, 72, 72)) return nullptr;
        h.current = pixelHeight;
    }

    Bitmap b;
    // light hinting like the interface: only vertically, the shapes keep
    // their widths
    if (FT_Load_Char(h.face, cp, FT_LOAD_TARGET_LIGHT | FT_LOAD_RENDER) == 0) {
        auto slot = h.face->glyph;
        auto& bm = slot->bitmap;
        int w = (int)bm.width, rows = (int)bm.rows;
        if (w > 0 && rows > 0 && bm.pixel_mode == FT_PIXEL_MODE_GRAY) {
            if (h.x + w + 1 > h.side) {
                h.x = 1;
                h.y += h.row + 1;
                h.row = 0;
            }
            if (h.y + rows + 1 > h.side) {
                if (h.side >= 2048 || w + 2 > 2048) {
                    // emptied when the batch is done with it
                    h.full = true;
                    return nullptr;
                }
                // twice as large, the glyphs keep their place
                std::vector<unsigned char> larger((size_t)2048 * 2048, 0);
                for (int r = 0; r < h.side; r++)
                    std::memcpy(&larger[(size_t)r * 2048], &h.pixels[(size_t)r * h.side], h.side);
                h.pixels = std::move(larger);
                h.side = 2048;
                h.resized = true;
                if (h.x + w + 1 > h.side) {
                    h.x = 1;
                    h.y += h.row + 1;
                    h.row = 0;
                }
            }
            // freetype rows are top down
            for (int r = 0; r < rows; r++)
                std::memcpy(&h.pixels[(size_t)(h.y + rows - 1 - r) * h.side + h.x], bm.buffer + r * bm.pitch, w);
            b.pos = { h.x, h.y };
            b.size = { w, rows };
            b.bearing = { slot->bitmap_left, slot->bitmap_top - rows };
            h.dirtyLo = std::min(h.dirtyLo, h.y);
            h.dirtyHi = std::max(h.dirtyHi, h.y + rows - 1);
            h.x += w + 1;
            h.row = std::max(h.row, rows);
        }
    }
    return &(h.cache[key] = b);
#else
    (void)cp;
    (void)pixelHeight;
    return nullptr;
#endif
}

GLuint Font::bitmapTexture()
{
#ifdef HD_HAS_FREETYPE
    if (!m_hinted) return 0;
    auto& h = *m_hinted;
    if (!h.texture) {
        glGenTextures(1, &h.texture);
        glBindTexture(GL_TEXTURE_2D, h.texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    } else {
        glBindTexture(GL_TEXTURE_2D, h.texture);
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    if (h.resized) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, h.side, h.side, 0, GL_RED, GL_UNSIGNED_BYTE, h.pixels.data());
        h.resized = false;
    } else if (h.dirtyHi >= h.dirtyLo) {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, h.dirtyLo, h.side, h.dirtyHi - h.dirtyLo + 1, GL_RED, GL_UNSIGNED_BYTE, &h.pixels[(size_t)h.dirtyLo * h.side]);
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    h.dirtyLo = INT_MAX;
    h.dirtyHi = -1;
    return h.texture;
#else
    return 0;
#endif
}

void Font::newBatch()
{
#ifdef HD_HAS_FREETYPE
    if (!m_hinted) return;
    auto& h = *m_hinted;
    h.budget = 256;
    if (h.full) {
        h.cache.clear();
        h.x = h.y = 1;
        h.row = 0;
        h.full = false;
    }
#endif
}

const Font::Glyph& Font::glyph(uint32_t cp) const
{
    auto it = glyphs.find(cp);
    return it == glyphs.end() ? fallback : it->second;
}

float Font::kerning(uint32_t a, uint32_t b) const
{
    return stbtt_GetCodepointKernAdvance(&info, (int)a, (int)b) * scale / px;
}

float Font::width(std::string_view s) const
{
    float w = 0;
    uint32_t prev = 0;
    for (size_t i = 0; i < s.size();) {
        uint32_t cp = decode(s, i);
        if (prev) w += kerning(prev, cp);
        w += glyph(cp).advance;
        prev = cp;
    }
    return w;
}

uint32_t Font::decode(std::string_view s, size_t& i)
{
    auto c = (unsigned char)s[i++];
    int extra = c >= 0xF0 ? 3 : c >= 0xE0 ? 2 : c >= 0xC0 ? 1 : 0;
    if (extra == 0 || i + extra > s.size()) return c;

    uint32_t cp = c & (0x3F >> extra);
    for (int k = 0; k < extra; k++) {
        auto n = (unsigned char)s[i + k];
        if ((n & 0xC0) != 0x80) return c;
        cp = (cp << 6) | (n & 0x3F);
    }
    i += extra;
    return cp;
}

}
