#version 330 core
#extension GL_ARB_explicit_uniform_location : require

in vec2 v_pos;
in vec2 v_uv;
flat in vec4 v_points;
flat in vec4 v_params;
flat in vec4 v_color;

layout(location = 1) uniform float u_px;
layout(location = 2) uniform sampler2D u_font;
// how far the font's distance fields reach, texture pixels
layout(location = 3) uniform float u_range;
// hinted glyphs of small text
layout(location = 4) uniform sampler2D u_bitmap;
// outlines of large text, quadratic curves monotonic in x and y. two texels
// per curve (p0 p1, p2), rows of 1024 (Font::curveRow)
layout(location = 5) uniform sampler2D u_curves;

vec4 curve_texel(int i)
{
    return texelFetch(u_curves, ivec2(i & 1023, i >> 10), 0);
}

out vec4 fragcolor;

float box_distance(vec2 p, vec2 half_size, float corner)
{
    vec2 q = abs(p) - half_size + corner;
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - corner;
}

// signed crossings of the ray from the origin to +x, each weighted by how
// much of the pixel (w wide) is left of it. their sum is the coverage of
// the pixel along the ray.
float crossings(vec2 p0, vec2 p1, vec2 p2, float w)
{
    // a monotonic curve crosses once if its ends are on both sides
    bool a = p0.y > 0.0, b = p2.y > 0.0;
    if (a == b) return 0.0;
    // y(t) = qa t^2 - 2 qb t + p0.y, the root in [0, 1]
    float qa = p0.y - 2.0 * p1.y + p2.y, qb = p0.y - p1.y;
    float t;
    if (abs(qa) < 1e-6 * (abs(p0.y) + abs(p2.y))) {
        t = p0.y / (p0.y - p2.y);
    } else {
        float s = sqrt(max(qb * qb - qa * p0.y, 0.0));
        float t0 = (qb - s) / qa, t1 = (qb + s) / qa;
        t = max(-t0, t0 - 1.0) < max(-t1, t1 - 1.0) ? t0 : t1;
    }
    t = clamp(t, 0.0, 1.0);
    float u = 1.0 - t;
    float x = u * u * p0.x + 2.0 * u * t * p1.x + t * t * p2.x;
    return (b ? 1.0 : -1.0) * clamp(x / w + 0.5, 0.0, 1.0);
}

void main()
{
    int type = int(v_params.w);

    if (type == 4) {
        // multi-channel distance field: the median is the distance, 0.5 on
        // the outline
        vec3 f = texture(u_font, v_uv).rgb;
        float d = max(min(f.r, f.g), min(max(f.r, f.g), f.b)) - 0.5;
        // the range in screen pixels, at least one so small text stays soft
        // instead of breaking up
        vec2 unit = vec2(u_range) / vec2(textureSize(u_font, 0));
        float range = max(0.5 * dot(unit, 1.0 / max(fwidth(v_uv), vec2(1e-7))), 1.0);
        float alpha = clamp(d * range + 0.5, 0.0, 1.0);
        if (alpha <= 0.0) discard;
        fragcolor = vec4(v_color.rgb, v_color.a * alpha);
        return;
    }

    if (type == 6) {
        // exact coverage from the outline: a horizontal and a vertical ray,
        // nonzero winding, smoothed across one pixel along each
        vec2 w = max(fwidth(v_uv), vec2(1e-9));
        int first = int(v_params.x), count = int(v_params.y);
        float h = 0.0, v = 0.0;
        for (int i = 0; i < count; i++) {
            vec4 a = curve_texel(2 * (first + i));
            vec2 c = curve_texel(2 * (first + i) + 1).xy;
            vec2 p0 = a.xy - v_uv, p1 = a.zw - v_uv, p2 = c - v_uv;
            h += crossings(p0, p1, p2, w.x);
            v += crossings(p0.yx, p1.yx, p2.yx, w.y);
        }
        float alpha = 0.5 * (min(abs(h), 1.0) + min(abs(v), 1.0));
        if (alpha <= 0.0) discard;
        fragcolor = vec4(v_color.rgb, v_color.a * alpha);
        return;
    }

    if (type == 5) {
        // hinted coverage, the quad sits on whole pixels. uv in texels.
        float alpha = texture(u_bitmap, v_uv / vec2(textureSize(u_bitmap, 0))).r;
        if (alpha <= 0.0) discard;
        fragcolor = vec4(v_color.rgb, v_color.a * alpha);
        return;
    }

    float d;
    // shapes thinner than a pixel are drawn a pixel wide but fainter. not
    // in proportion, zoomed out drawings would get too faint to see.
    float fade = 1.0;

    if (type <= 1) {
        vec2 a = v_points.xy, b = v_points.zw;
        float hw = max(v_params.x, 0.5 * u_px);
        fade = sqrt(v_params.x / hw);
        vec2 pa = v_pos - a, ba = b - a;
        if (type == 0) {
            float h = clamp(dot(pa, ba) / max(dot(ba, ba), 1e-12), 0.0, 1.0);
            d = length(pa - ba * h) - hw;
        } else {
            float len = length(ba);
            vec2 dir = len > 0.0 ? ba / len : vec2(1, 0);
            vec2 local = vec2(dot(pa, dir) - len / 2.0, dot(pa, vec2(-dir.y, dir.x)));
            d = box_distance(local, vec2(len / 2.0, hw), 0.0);
        }
    } else {
        if (type == 2)
            d = length(v_pos - v_points.xy) - v_params.x;
        else
            d = box_distance(v_pos - (v_points.xy + v_points.zw) / 2.0, (v_points.zw - v_points.xy) / 2.0, v_params.z);

        // outline
        if (v_params.y > 0.0) {
            float hw = max(v_params.y, 0.5 * u_px);
            fade = sqrt(v_params.y / hw);
            d = abs(d) - hw;
        }
    }

    float alpha = clamp(0.5 - d / u_px, 0.0, 1.0) * fade;
    if (alpha <= 0.0) discard;
    fragcolor = vec4(v_color.rgb, v_color.a * alpha);
}
