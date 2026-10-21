#version 330 core
#extension GL_ARB_explicit_uniform_location : require

layout(location=1) uniform vec2 c = vec2(0);

void main()
{
    gl_Position = vec4(c, 0, 1);
}
