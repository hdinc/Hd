#version 450 core
#extension GL_ARB_explicit_uniform_location : enable

struct ball{
    vec2 loc[2][10];
    vec2 speed[2][10];
};

layout(location=0) in vec2 pos;
layout(location=1) uniform mat4 VP = mat4(1);
layout(location=2) uniform float circle_size = 1;
layout(location=3) uniform int index = 1;

layout(std430, binding = 3) buffer locations
{
    ball balls;
};

void main()
{
    mat4 M = mat4(mat2(circle_size));
    M += mat4(vec4(0),vec4(0),vec4(0),vec4(balls.loc[index][gl_InstanceID],0,1));
    gl_Position = (VP * M) * vec4(pos,0,1);
}
