#version 450 core

uniform float deltatime;
uniform int count;
uniform int swap;
uniform borderw;
uniform borderh;
uniform radius;

struct ball {
    vec2 location[2];
    vec2 speed[2];
};

layout(std430, binding = 3) buffer layoutName
{
    ball balls[];
};

void main()
{

    for(unsigned i=0;i<count;i++) {
        balls[i].location[swap] += balls[i].speed[swap] * deltatime;

        if((balls[i].location.x>borderw/2-radius && balls[i].speed.x>0)||(balls[i].location.x<-(borderw/2-radius) && balls[i].speed.x<0)) {
            balls[i].speed.x=-balls[i].speed.x;
        }
        if((balls[i].location.y>borderh/2-radius && balls[i].speed.y>0)||(balls[i].location.y<-(borderh/2-radius) && balls[i].speed.y<0)) {
            balls[i].speed.y=-balls[i].speed.y;
        }
    }

    glm::vec2 ballAx,ballAy,ballBx,ballBy,AB,nAB;
    for (unsigned i = 0; i < count - 1 ; i++) {
        for(unsigned j = i + 1; j < count ; j++) {
            if(glm::length(balls[i].loc-balls[j].loc)< 2 * ball::radius) {

                AB = glm::normalize(balls[j].loc - balls[i].loc);
                ballAx = AB * glm::dot(balls[i].speed,AB);
                ballBx = AB * glm::dot(balls[j].speed,AB);
                nAB = rotate90(AB);
                ballAy = nAB * glm::dot(balls[i].speed,nAB);
                ballBy = nAB * glm::dot(balls[j].speed,nAB);

                if(glm::dot(ballAx,ballBx) > 0)
                {
                    if(glm::dot(AB,ballAx) > 0) {
                        if(glm::length(ballAx) > glm::length(ballBx)) {
                            balls[i].speed = ballBx+ballAy;
                            balls[j].speed = ballAx+ballBy;
                        }
                    }  else {
                        if(glm::length(ballBx) > glm::length(ballAx)) {
                            balls[i].speed = ballBx+ballAy;
                            balls[j].speed = ballAx+ballBy;
                        }
                    }
                }
                else if(glm::dot(ballAx,AB) > 0){
                    balls[i].speed = ballBx+ballAy;
                    balls[j].speed = ballAx+ballBy;
                }
            }
        }
    }
}


void drawBalls(ball balls[],unsigned count)
{

    for(unsigned i=0;i<count-1;i++) {
        ball::c = glm::vec4(.4,.4,.4,1.f);
        balls[i].draw();
    }
    ball::c = glm::vec4(1,.2,.2,1.f);
    balls[count-1].draw();
}
