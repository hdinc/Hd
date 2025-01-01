#version 450 core

out vec4 fragcolor;

layout(location = 3) uniform vec4 color = vec4(1,1,1,1);

void main()
{
   fragcolor = color;
}
