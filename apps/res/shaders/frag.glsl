#version 330 core

uniform vec3 COLOR = vec3(1.0);
out vec4 c;

void main()
{
    c = vec4(COLOR,1.0f);
}
