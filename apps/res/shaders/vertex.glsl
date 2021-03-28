#version 450 core

layout(location=0) in vec3 a_position;
layout(location=1) in vec3 a_normal;
layout(location=2) in vec2 a_texcoord;

layout(location=0) uniform mat4 u_mvp = mat4(1);

out vec3 normal;
out vec2 texcoord;

void main()
{
    gl_Position = u_mvp * vec4(a_position,1);
    normal = a_normal;
    texcoord = a_texcoord;
}
