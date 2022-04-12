#version 450 core

layout(location=0) uniform mat4 u_mvp = mat4(1);
layout(location=1) uniform vec3 u_points[2];

void main() {
    gl_Position = u_mvp * vec4(u_points[gl_VertexID],1.0);
}
