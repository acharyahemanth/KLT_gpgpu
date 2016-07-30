#version 330 core

layout(location=0) out float output1;
layout(location=1) out float output2;

uniform sampler2D ip_texture_sampler;

void main(){
    output1 = 5.0;
    output2 = 10.0;
    
    vec2 coord = gl_FragCoord.xy;
    vec2 uv = vec2(float(coord.x)/float(3),
                   float(coord.y)/float(3));
    output1 = texture(ip_texture_sampler, uv).x;
}