#version 450 core

in vec2 texcoord;
out vec4 fragcolor;

uniform sampler2D tex;

void main()
{
   fragcolor = texture(tex,texcoord);
}
