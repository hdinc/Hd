#version 450 core

in vec2 v_pos;
in vec2 v_uv;
flat in vec4 v_points;
flat in vec4 v_params;
flat in vec4 v_color;

layout(location = 1) uniform float u_px;
layout(location = 2) uniform sampler2D u_font;

out vec4 fragcolor;

float box_distance(vec2 p, vec2 half_size, float corner)
{
    vec2 q = abs(p) - half_size + corner;
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - corner;
}

void main()
{
    int type = int(v_params.w);

    if (type == 4) {
        // distance field, 0.5 on the outline
        float v = texture(u_font, v_uv).r;
        float w = max(fwidth(v), 1e-4);
        float alpha = smoothstep(0.5 - w, 0.5 + w, v);
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
