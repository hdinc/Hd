#version 450 core

out vec4 color;

uniform vec2 viewport = vec2(1920,1080);

const float iteration = 20;
const float zoom = .2;
const vec2 zoomloc = vec2(0,0);

vec2 square(vec2 v);

void main()
{
    vec2 loc = gl_FragCoord.xy/viewport;
    loc = vec2(loc.x-.5,-loc.y+.5);
    loc -= zoomloc;
    loc/=zoom;

    vec2 z = vec2(0);
    for(float i = 0; i < iteration; i++) {
        z = square(z) + loc;

        if(length(z)>2.) {
            color = vec4(vec3(1. - i/iteration, 1.- i/iteration, 1.- i/iteration),1.);
            return;
        }
    }

    color = vec4(0,0,0,1);

}

vec2 square(vec2 v)
{
    return vec2(v.x*v.x-v.y*v.y,2*v.x*v.y);
}
