#include "Msdf.h"

#include <msdfgen.h>
#include <core/ShapeDistanceFinder.h>

#include <algorithm>
#include <cmath>
#include <thread>

namespace Hd::msdf {

namespace {

// the outline of a glyph from stb_truetype, false if it has none
bool outline(const stbtt_fontinfo& info, int glyph, msdfgen::Shape& out)
{
    stbtt_vertex* v = nullptr;
    int n = stbtt_GetGlyphShape(&info, glyph, &v);

    msdfgen::Contour* c = nullptr;
    msdfgen::Point2 first, p;
    auto same = [](msdfgen::Point2 a, msdfgen::Point2 b) { return a.x == b.x && a.y == b.y; };
    auto close = [&] {
        if (c && !same(p, first)) c->addEdge(msdfgen::EdgeHolder(p, first));
    };

    for (int k = 0; k < n; k++) {
        auto& e = v[k];
        msdfgen::Point2 q(e.x, e.y);
        if (e.type == STBTT_vmove) {
            close();
            c = &out.addContour();
            first = q;
        } else if (c && !same(p, q)) {
            if (e.type == STBTT_vline)
                c->addEdge(msdfgen::EdgeHolder(p, q));
            else if (e.type == STBTT_vcurve)
                c->addEdge(msdfgen::EdgeHolder(p, msdfgen::Point2(e.cx, e.cy), q));
            else if (e.type == STBTT_vcubic)
                c->addEdge(msdfgen::EdgeHolder(p, msdfgen::Point2(e.cx, e.cy), msdfgen::Point2(e.cx1, e.cy1), q));
        }
        p = q;
    }
    close();
    stbtt_FreeShape(&info, v);

    out.contours.erase(std::remove_if(out.contours.begin(), out.contours.end(), [](auto& c) { return c.edges.empty(); }), out.contours.end());
    return !out.contours.empty();
}

}

Field generate(const stbtt_fontinfo& info, int glyph, float scale, float range)
{
    Field f;
    msdfgen::Shape shape;
    if (!outline(info, glyph, shape)) return f;
    shape.normalize();
    auto b = shape.getBounds();
    if (b.l >= b.r || b.b >= b.t) return f;

    // outer contours must wind so that the outside is negative. truetype and
    // cff wind opposite ways.
    msdfgen::Point2 outside(b.l - (b.r - b.l) - 1, b.b - (b.t - b.b) - 1);
    if (msdfgen::SimpleTrueShapeDistanceFinder::oneShotDistance(shape, outside) > 0)
        for (auto& c : shape.contours)
            c.reverse();
    msdfgen::edgeColoringInkTrap(shape, 3.0);

    int pad = (int)std::ceil(range / 2) + 1;
    f.w = (int)std::ceil((b.r - b.l) * scale) + 2 * pad;
    f.h = (int)std::ceil((b.t - b.b) * scale) + 2 * pad;
    msdfgen::Vector2 translate(pad / scale - b.l, pad / scale - b.b);
    msdfgen::Projection projection(msdfgen::Vector2(scale), translate);
    msdfgen::SDFTransformation transformation(projection, msdfgen::DistanceMapping(msdfgen::Range(range / scale)));

    // like msdfgen's scanline mode: the sign from a nonzero fill of the
    // outline, then error correction
    msdfgen::MSDFGeneratorConfig config(true);
    auto correction = config;
    config.errorCorrection.mode = msdfgen::ErrorCorrectionConfig::DISABLED;
    msdfgen::Bitmap<float, 3> bitmap(f.w, f.h);
    msdfgen::generateMSDF(bitmap, shape, transformation, config);
    msdfgen::distanceSignCorrection(bitmap, shape, projection, msdfgen::FILL_NONZERO);
    msdfgen::msdfErrorCorrection(bitmap, shape, transformation, correction);

    f.origin = glm::vec2(-translate.x, -translate.y) * scale;
    f.rgb.resize(f.w * f.h * 3);
    for (int y = 0; y < f.h; y++)
        for (int x = 0; x < f.w; x++)
            for (int k = 0; k < 3; k++)
                f.rgb[(y * f.w + x) * 3 + k] = msdfgen::pixelFloatToByte(bitmap(x, y)[k]);
    return f;
}

Atlas pack(const stbtt_fontinfo& info, const std::vector<uint32_t>& codepoints, float scale, float range)
{
    // the codepoints the font has
    std::vector<uint32_t> cps;
    for (auto cp : codepoints)
        if (stbtt_FindGlyphIndex(&info, (int)cp)) cps.push_back(cp);

    // fields on all cores, each shape on its own
    std::vector<Field> fields(cps.size());
    unsigned threads = std::clamp(std::thread::hardware_concurrency(), 1u, 8u);
    std::vector<std::thread> pool;
    for (unsigned t = 0; t < threads; t++)
        pool.emplace_back([&, t] {
            for (size_t k = t; k < cps.size(); k += threads)
                fields[k] = generate(info, stbtt_FindGlyphIndex(&info, (int)cps[k]), scale, range);
        });
    for (auto& t : pool)
        t.join();

    // shelves, tallest first
    std::vector<size_t> order(cps.size());
    for (size_t k = 0; k < order.size(); k++)
        order[k] = k;
    std::sort(order.begin(), order.end(), [&](size_t a, size_t b) { return fields[a].h > fields[b].h; });

    Atlas a;
    for (int size : { 1024, 2048, 4096 }) {
        a.size = size;
        a.glyphs.clear();
        bool fits = true;
        int x = 1, y = 1, row = 0;
        for (auto k : order) {
            auto& f = fields[k];
            Atlas::Glyph g;
            g.origin = f.origin;
            if (f.w > 0) {
                if (x + f.w + 1 > size) {
                    x = 1;
                    y += row + 1;
                    row = 0;
                }
                if (y + f.h + 1 > size) {
                    fits = false;
                    if (size < 4096) break;
                    continue;
                }
                g.pos = { x, y };
                g.size = { f.w, f.h };
                x += f.w + 1;
                row = std::max(row, f.h);
            }
            a.glyphs[cps[k]] = g;
        }
        if (fits || size == 4096) break;
    }

    a.rgb.assign((size_t)a.size * a.size * 3, 0);
    for (size_t k = 0; k < cps.size(); k++) {
        auto it = a.glyphs.find(cps[k]);
        if (it == a.glyphs.end() || it->second.size.x == 0) continue;
        auto& f = fields[k];
        auto pos = it->second.pos;
        for (int r = 0; r < f.h; r++)
            std::copy_n(&f.rgb[r * f.w * 3], f.w * 3, &a.rgb[((size_t)(pos.y + r) * a.size + pos.x) * 3]);
    }
    return a;
}

}
