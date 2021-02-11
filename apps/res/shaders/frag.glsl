#version 450 core

layout(location = 3) uniform vec4 u_color = vec4(1);
out vec4 c;

void main()
{
    c = u_color;
}
