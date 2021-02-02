#version 450 core

layout(location = 1) uniform vec3 u_color = vec3(1.0);
out vec4 c;

void main()
{
    c = vec4(u_color,1.0f);
}
