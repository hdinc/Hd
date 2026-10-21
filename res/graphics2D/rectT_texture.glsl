#version 330 core
#extension GL_ARB_explicit_uniform_location : require

in vec2 texcoord;
out vec4 fragcolor;

layout(location = 5) uniform sampler2D u_tex;

void main()
{
   fragcolor = texture(u_tex,texcoord);
}
