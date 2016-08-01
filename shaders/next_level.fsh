#version 330 core
precision highp float;

//Uniforms
uniform int num_points;

//Texture samplers
uniform sampler2D srcpts_texture_sampler;
uniform sampler2D predpts_texture_sampler;

//Outputs : src_next = src*2, pred_next = pred*2
layout(location=0) out vec2 src_next;
layout(location=1) out vec2 pred_next;

void main(){
    
    //glFragcoord gives the center of the pixel, ie (0,0) -> (0.5, 0.5)
    vec2 coord = gl_FragCoord.xy - vec2(0.5,0.5);
    
    //Current source and corresponding prediction corners
    vec2 tex_uv = vec2(float(coord.x) / float(num_points),
                       0.0).xy;
    vec2 curr_source_corner = texture(srcpts_texture_sampler, tex_uv).xy;
    vec2 curr_pred_corner = texture(predpts_texture_sampler, tex_uv).xy;
    
    //Multiply by 2 to project to next level
    src_next = 2*curr_source_corner;
    pred_next = 2*curr_pred_corner;
    
}