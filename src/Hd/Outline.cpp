#include "Outline.h"

#include <cmath>

namespace Hd::outline {

namespace {

glm::vec2 at(const Curve& c, float t)
{
    float u = 1 - t;
    return u * u * c.p0 + 2 * u * t * c.p1 + t * t * c.p2;
}

// the parts before and after t
void split(const Curve& c, float t, Curve& a, Curve& b)
{
    glm::vec2 m0 = glm::mix(c.p0, c.p1, t), m1 = glm::mix(c.p1, c.p2, t), m = glm::mix(m0, m1, t);
    a = { c.p0, m0, m };
    b = { m, m1, c.p2 };
}

// where the derivative of one coordinate is 0, -1 if not inside the curve
float extremum(float p0, float p1, float p2)
{
    float d = p0 - 2 * p1 + p2;
    if (d == 0) return -1;
    float t = (p0 - p1) / d;
    return t > 1e-4f && t < 1 - 1e-4f ? t : -1;
}

void add(std::vector<Curve>& out, const Curve& c)
{
    if (c.p0 == c.p2 && c.p1 == c.p0) return;
    // split at the extrema of x, then of y
    float tx = extremum(c.p0.x, c.p1.x, c.p2.x);
    if (tx > 0) {
        Curve a, b;
        split(c, tx, a, b);
        // exactly monotonic: the control point onto the extremum
        a.p1.x = a.p2.x;
        b.p1.x = b.p0.x;
        add(out, a);
        add(out, b);
        return;
    }
    float ty = extremum(c.p0.y, c.p1.y, c.p2.y);
    if (ty > 0) {
        Curve a, b;
        split(c, ty, a, b);
        a.p1.y = a.p2.y;
        b.p1.y = b.p0.y;
        add(out, a);
        add(out, b);
        return;
    }
    out.push_back(c);
}

}

std::vector<Curve> curves(const stbtt_fontinfo& info, int glyph)
{
    stbtt_vertex* v = nullptr;
    int n = stbtt_GetGlyphShape(&info, glyph, &v);

    std::vector<Curve> r;
    glm::vec2 p { 0 }, first { 0 };
    bool open = false;
    auto line = [&](glm::vec2 a, glm::vec2 b) {
        if (a != b) add(r, { a, (a + b) * 0.5f, b });
    };

    for (int k = 0; k < n; k++) {
        auto& e = v[k];
        glm::vec2 q(e.x, e.y);
        if (e.type == STBTT_vmove) {
            if (open) line(p, first);
            first = q;
            open = true;
        } else if (e.type == STBTT_vline) {
            line(p, q);
        } else if (e.type == STBTT_vcurve) {
            add(r, { p, glm::vec2(e.cx, e.cy), q });
        } else if (e.type == STBTT_vcubic) {
            // four quadratics, each with the control point where the
            // tangents of its part of the cubic meet (approximately)
            glm::vec2 c1(e.cx, e.cy), c2(e.cx1, e.cy1);
            auto cubic = [&](float t) {
                float u = 1 - t;
                return u * u * u * p + 3 * u * u * t * c1 + 3 * u * t * t * c2 + t * t * t * q;
            };
            auto tangent = [&](float t) {
                float u = 1 - t;
                return 3 * u * u * (c1 - p) + 6 * u * t * (c2 - c1) + 3 * t * t * (q - c2);
            };
            for (int i = 0; i < 4; i++) {
                float t0 = i / 4.0f, t1 = (i + 1) / 4.0f;
                glm::vec2 a = cubic(t0), b = cubic(t1);
                glm::vec2 da = tangent(t0) * (t1 - t0), db = tangent(t1) * (t1 - t0);
                // midpoint of the two tangent ends, exact for quadratics
                glm::vec2 c = (a + da * 0.5f + b - db * 0.5f) * 0.5f;
                add(r, { a, c, b });
            }
        }
        p = q;
    }
    if (open) line(p, first);
    stbtt_FreeShape(&info, v);
    return r;
}

int winding(const std::vector<Curve>& curves, glm::vec2 p)
{
    int w = 0;
    for (auto& c : curves) {
        bool a = c.p0.y > p.y, b = c.p2.y > p.y;
        if (a == b) continue;
        // the one crossing of the monotonic curve
        float y0 = c.p0.y - p.y, y1 = c.p1.y - p.y, y2 = c.p2.y - p.y;
        float qa = y0 - 2 * y1 + y2, qb = y0 - y1;
        // y(t) = qa t^2 - 2 qb t + y0, the root in [0, 1]
        float t;
        if (std::abs(qa) < 1e-6f * (std::abs(y0) + std::abs(y2))) {
            t = y0 / (y0 - y2);
        } else {
            float s = std::sqrt(std::max(qb * qb - qa * y0, 0.0f));
            float t0 = (qb - s) / qa, t1 = (qb + s) / qa;
            auto outside = [](float t) { return std::max(-t, t - 1); };
            t = outside(t0) < outside(t1) ? t0 : t1;
        }
        t = glm::clamp(t, 0.0f, 1.0f);
        if (at(c, t).x > p.x) w += b ? 1 : -1;
    }
    return w;
}

}
