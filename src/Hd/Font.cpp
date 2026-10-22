#include <Hd/Font.h>
#include <Hd/Resource.h>

#include <cstring>
#include <stdexcept>

namespace Hd {

namespace {

constexpr int atlas_size = 1024;

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

    // distance field reaches `padding` pixels around the outline
    const int padding = px / 8;
    const float dist_scale = 128.0f / padding;

    std::vector<unsigned char> atlas(atlas_size * atlas_size, 0);
    int x = 1, y = 1, row = 0;

    for (uint32_t cp : codepoints()) {
        if (!stbtt_FindGlyphIndex(&info, (int)cp)) continue;

        int adv, lsb;
        stbtt_GetCodepointHMetrics(&info, (int)cp, &adv, &lsb);

        Glyph g {};
        g.advance = adv * scale / px;

        int w = 0, h = 0, xoff = 0, yoff = 0;
        unsigned char* sdf = stbtt_GetCodepointSDF(&info, scale, (int)cp, padding, 128, dist_scale, &w, &h, &xoff, &yoff);
        if (sdf) {
            if (x + w + 1 > atlas_size) {
                x = 1;
                y += row + 1;
                row = 0;
            }
            if (y + h + 1 > atlas_size) {
                stbtt_FreeSDF(sdf, nullptr);
                break;
            }
            for (int r = 0; r < h; r++)
                std::memcpy(&atlas[(y + r) * atlas_size + x], sdf + r * w, w);
            stbtt_FreeSDF(sdf, nullptr);

            g.uv0 = glm::vec2(x, y + h) / float(atlas_size);
            g.uv1 = glm::vec2(x + w, y) / float(atlas_size);
            g.min = glm::vec2(xoff, -(yoff + h)) / float(px);
            g.max = glm::vec2(xoff + w, -yoff) / float(px);

            x += w + 1;
            row = std::max(row, h);
        }
        glyphs[cp] = g;
    }

    fallback = glyphs.count('?') ? glyphs['?'] : Glyph {};

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, atlas_size, atlas_size, 0, GL_RED, GL_UNSIGNED_BYTE, atlas.data());
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

Font::~Font()
{
    if (m_texture) glDeleteTextures(1, &m_texture);
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
