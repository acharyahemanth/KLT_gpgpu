precision highp float;

//Uniforms
uniform int num_points;
uniform int window_size;
uniform int image_width;
uniform int image_height;
uniform int pyramid_level;

//Texture samplers
uniform sampler2D A_texture_sampler;
uniform sampler2D b_texture_sampler;
uniform sampler2D predpts_texture_sampler;
//uniform sampler2D w_texture_sampler;

//Outputs :
//a) tracked_pt = pred_pt + (AtA).inv()*At*b
//b) weights : 1 if point is in, 0 if its out
layout(location=0) out vec2 tracked_pt;
layout(location=1) out float point_shift_delta;

void main(){
    image_width;
    image_height;
    
    
    //glFragcoord gives the center of the pixel, ie (0,0) -> (0.5, 0.5)
    //vec2 coord = gl_FragCoord.xy - vec2(0.5,0.5);
    
    //Go through all rows of current point, assemble the elements of A_t*A and A_t*b matrices
    float ata_00, ata_01, ata_11;
    float atb_00, atb_10;
    ata_00 = ata_01 = ata_11 = atb_00 = atb_10 = 0.0;
    vec2 tex_uv, a_i, b_i;
    int num_rows = window_size*window_size;
    for(int r=0; r<num_rows; r++){
        tex_uv = vec2(float(gl_FragCoord.x) / float(num_points),
                      float(r) / float(num_rows)).xy;
        vec2 a_i = texture(A_texture_sampler, tex_uv).xy;
        float b_i = texture(b_texture_sampler, tex_uv).x;
        ata_00 = ata_00 + (a_i.x*a_i.x);
        ata_11 = ata_11 + (a_i.y*a_i.y);
        ata_01 = ata_01 + (a_i.x*a_i.y);
        atb_00 = atb_00 + (a_i.x*b_i);
        atb_10 = atb_10 + (a_i.y*b_i);
    }
    
    //Populate the matrices
    mat2 ata;
    vec2 atb;
    ata[0][0] = ata_00;
    ata[1][1] = ata_11;
    ata[0][1] = ata_01;
    ata[1][0] = ata_01;
    atb.x = atb_00;
    atb.y = atb_10;
    
    //Calculate the output delta from prediction
    vec2 delta = inverse(ata)*atb;
    
    //tracked_pt = prediction + delta
    tex_uv = vec2(float(gl_FragCoord.x) / float(num_points),
                  0.0).xy;
    vec2 pred_corner = texture(predpts_texture_sampler, tex_uv).xy;
    tracked_pt = pred_corner.xy + delta.xy;
    
    //check if its within image boundary : todo!
//    w = 1.0;
    
    int shift = (1<<pyramid_level);
    vec2 delta_level0 = vec2(float(delta.x*float(shift)), float(delta.y*float(shift))).xy;
    point_shift_delta = length(delta_level0);
    point_shift_delta = length(delta);
}