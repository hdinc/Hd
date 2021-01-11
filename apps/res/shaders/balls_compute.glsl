#version 450 core
#extension GL_ARB_explicit_uniform_location : enable

struct ball{
    vec2 loc[2][10];
    vec2 speed[2][10];
};

layout(std430, binding = 3) buffer locations
{
    ball balls;
};

layout(location=1) uniform int index;
layout(location=2) uniform vec2 border;
layout(location=3) uniform float radius;
layout(location=4) uniform float dt;

layout(local_size_x = 10 , local_size_y = 1, local_size_z = 1) in;

void borderbounce();
void ballbounce();
void updateloc();
vec2 rotate90(vec2 x);

void main()
{
    updateloc();
    borderbounce();
    ballbounce();
}


void borderbounce()
{
    vec2 loc = balls.loc[index][gl_LocalInvocationID.x];
    vec2 speed = balls.speed[index][gl_LocalInvocationID.x];

    if ((loc.x > border.x / 2 - radius && speed.x > 0) ||
            (loc.x < -(border.x / 2 - radius) && speed.x < 0)) {
        speed.x = -speed.x;
    }
    if ((loc.y > border.y / 2 - radius && speed.y > 0) ||
            (loc.y < -(border.y / 2 - radius) && speed.y < 0)) {
        speed.y = -speed.y;
    }

    balls.speed[index][gl_LocalInvocationID.x] = speed;
}

void ballbounce()
{
    vec2 ballAx, ballAy, ballBx, ballBy, AB, nAB;
    vec2 loc = balls.loc[1-index][gl_LocalInvocationID.x];
    vec2 speed = balls.speed[1-index][gl_LocalInvocationID.x];

    for (uint j = 0; j < 10; j++) {
        if(j == gl_LocalInvocationID.x) continue;

        vec2 speedj = balls.speed[1-index][j];
        vec2 locj = balls.loc[1-index][j];

        if (length(loc - locj) < 2 * radius) {

            AB = normalize(locj - loc);
            ballAx = AB * dot(speed, AB);
            ballBx = AB * dot(speedj, AB);
            nAB = rotate90(AB);
            ballAy = nAB * dot(speed, nAB);
            ballBy = nAB * dot(speedj, nAB);

            if (dot(ballAx, ballBx) > 0) {
                if (dot(AB, ballAx) > 0) {
                    if (length(ballAx) > length(ballBx)) {
                        balls.speed[index][gl_LocalInvocationID.x] = ballBx + ballAy;
                    }
                } else {
                    if (length(ballBx) > length(ballAx)) {
                        balls.speed[index][gl_LocalInvocationID.x] = ballBx + ballAy;
                    }
                }
            } else if (dot(ballAx, AB) > 0) {
                balls.speed[index][gl_LocalInvocationID.x] = ballBx + ballAy;
            }
        }
    }
}

void updateloc()
{
    vec2 loc = balls.loc[1-index][gl_LocalInvocationID.x];
    vec2 speed = balls.speed[1-index][gl_LocalInvocationID.x];

    balls.loc[index][gl_LocalInvocationID.x] = loc + speed*dt; 
    balls.speed[index][gl_LocalInvocationID.x] = speed;
}

vec2 rotate90(vec2 x)
{
    vec2 r;

    r.x = -x.y;
    r.y = x.x;

    return r;
}
