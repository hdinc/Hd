#version 330 core

uniform mat4 MVP = mat4(1);
uniform vec2 POINTS[2];

void main() {
    gl_Position = MVP * vec4(POINTS[gl_VertexID],0.0,1.0);
}
