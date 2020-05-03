#version 450 core

layout(location=0) in vec4 a_location;
layout(location=1) in vec2 a_texcoord;

uniform mat4 MVP = mat4(1);

out vec2 texcoord;

void main()
{
    gl_Position = MVP * a_location;
    texcoord = a_texcoord;
}
