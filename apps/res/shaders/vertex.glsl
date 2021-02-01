#version 450 core

layout(location=0) in vec3 a_location;
layout(location=1) in vec2 a_texcoord;

uniform mat4 MVP = mat4(1);

out vec2 texcoord;

void main()
{
    gl_Position = MVP * vec4(a_location,1);
    texcoord = a_texcoord;
}
