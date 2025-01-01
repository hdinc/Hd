#version 450 core

layout(location=1) uniform vec2 c = vec2(0);

void main()
{
    gl_Position = vec4(c, 0, 1);
}
