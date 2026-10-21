#version 330 core
#extension GL_ARB_explicit_uniform_location : require

// one instance per shape, the quad covering it is made here. see Renderer2D.

layout(location = 0) in vec4 a_points;
layout(location = 1) in vec4 a_params; // half width or radius, outline half width, corner, type
layout(location = 2) in vec4 a_color;
layout(location = 3) in vec4 a_uv;

layout(location = 0) uniform mat4 u_vp;
layout(location = 1) uniform float u_px; // world units per pixel

out vec2 v_pos;
out vec2 v_uv;
flat out vec4 v_points;
flat out vec4 v_params;
flat out vec4 v_color;

void main()
{
    vec2 corner = vec2(gl_VertexID & 1, gl_VertexID >> 1);
    int type = int(a_params.w);
    // room for the smoothed edge
    float pad = 1.5 * u_px;

    vec2 p;
    if (type <= 1) {
        vec2 a = a_points.xy, b = a_points.zw;
        vec2 d = b - a;
        float len = length(d);
        vec2 dir = len > 0.0 ? d / len : vec2(1, 0);
        vec2 n = vec2(-dir.y, dir.x);
        float hw = max(a_params.x, 0.5 * u_px) + pad;
        float ext = type == 0 ? hw : pad;
        p = a + dir * mix(-ext, len + ext, corner.x) + n * mix(-hw, hw, corner.y);
    } else if (type == 2) {
        float r = a_params.x + max(a_params.y, 0.5 * u_px) + pad;
        p = a_points.xy + mix(vec2(-r), vec2(r), corner);
    } else if (type == 3) {
        float e = max(a_params.y, 0.5 * u_px) + pad;
        p = mix(a_points.xy - e, a_points.zw + e, corner);
    } else {
        p = mix(a_points.xy, a_points.zw, corner);
        v_uv = mix(a_uv.xy, a_uv.zw, corner);
    }

    v_pos = p;
    v_points = a_points;
    v_params = a_params;
    v_color = a_color;
    gl_Position = u_vp * vec4(p, 0, 1);
}
