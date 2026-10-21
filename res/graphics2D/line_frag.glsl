#version 330 core
#extension GL_ARB_explicit_uniform_location : require

out vec4 fragcolor;

layout(location = 3) uniform vec4 u_color = vec4(1,1,1,1);

void main()
{
   fragcolor = u_color;
}
