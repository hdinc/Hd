#version 330 core
#extension GL_ARB_explicit_uniform_location : require

in vec2 texcoord;
out vec4 fragcolor;

layout(location=1) uniform sampler2D u_tex;
layout(location=6) uniform vec3 color = vec3(0);

void main()
{
   float alpha = texture(u_tex,texcoord).x;
   fragcolor = vec4(color, alpha);
}
