#version 450 core

out vec4 fragcolor;

layout(location = 5) uniform vec4 u_color = vec4(1,1,1,1);

void main()
{
   fragcolor = u_color;
}
