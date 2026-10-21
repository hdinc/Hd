#version 330 core
#extension GL_ARB_explicit_uniform_location : require

layout(location=0) uniform mat4 mvp = mat4(1);
layout(location=1) uniform vec2 pts[4] = vec2[4] ( vec2(0), vec2(0), vec2(0), vec2(0) );

void main()
{
    vec2 p = pts[gl_VertexID];

    gl_Position = mvp * vec4(p, 0, 1);
}
