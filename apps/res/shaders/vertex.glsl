#version 450 core

layout(location=0) in vec3 a_position;
layout(location=1) in vec2 a_texcoord;

layout(location=0) uniform mat4 u_mvp = mat4(1);

out vec2 texcoord;

void main()
{
    gl_Position = u_mvp * vec4(a_position,1);
    texcoord = a_texcoord;
}
