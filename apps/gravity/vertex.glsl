#version 450 core


struct ball {
    vec2 location[2];
    vec2 speed[2];
};

layout(location=0) in vec2 pos;
layout(std430,binding=3) buffer bals
{
    ball balls[];
};

void main()
{
    //gl_Position = vec4((pos/3.0+balls[gl_InstanceID].location[0])/25.0,0.5,1.);
    gl_Position = vec4(pos/50.0+balls[gl_InstanceID].location[0]/1.0,0.5,1.);
}
