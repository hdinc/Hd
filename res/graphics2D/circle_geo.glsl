#version 450 core

layout (points) in;
layout (triangle_strip, max_vertices = 128) out;

layout(location=2) uniform float r;
layout(location=0) uniform mat4 vp;

vec2 rotate(vec2 v, float a) {
	float s = sin(a);
	float c = cos(a);
	mat2 m = mat2(c, -s, s, c);
	return m * v;
}

void main() {

    vec2 c = gl_in[0].gl_Position.xy;
    vec2 d = vec2(0, r);

    for(int i = 0; i < 20; i++) {

        float a0 = radians(i / 20.0 * 360.0);
        float a1 = radians((i + 1) / 20.0 * 360.0);

        gl_Position = vp * vec4(c, 0, 1);
        EmitVertex();

        gl_Position = vp * vec4(c + rotate(d, a0), 0, 1);
        EmitVertex();

        gl_Position = vp * vec4(c + rotate(d, a1), 0, 1);
        EmitVertex();

        EndPrimitive();
    }

}
