#version 450 core

layout(location=0) in vec4 location;
uniform mat4 MVP = mat4(1);

void main()
{
    gl_Position = MVP * location;
}
