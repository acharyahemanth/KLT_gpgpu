#version 330 core

//Uniforms
uniform int num_points;
uniform int window_size;
uniform int image_width;
uniform int image_height;
uniform int pyramid_level;

//Texture samplers
uniform sampler2D srcimage_texture_sampler;
uniform sampler2D srcpts_texture_sampler;

//Outputs : a/b is a vec2 matrix of size window_size*window_size*#points
//To calculate a mat for a particular point, you need to deserialize a particular column of A
//a/w fills in one element in the a/b vec2 matrices
layout(location=0) out vec2 a;
layout(location=1) out float w;

//Returns the pixel value at the pel specified at the pyramid level specified
float getPel(vec2 pel){
    int shift = (1<<pyramid_level);
    vec2 pel_level0 = vec2(float(pel.x*shift), float(pel.y*shift)).xy;
    vec2 tex_uv = vec2(float(pel_level0.x) / float(image_width),
                       float(pel_level0.y) / float(image_height)).xy;
    return texture(srcimage_texture_sampler, tex_uv).x;
}

//Function calculates the gradient of the source image at the pixel on the pyramid level specified
vec2 getGradient(vec2 pel){
    //Get the pels within the 3x3 Sobel window
    float p_00 = getPel(pel+vec2(-1,-1));
    float p_01 = getPel(pel+vec2( 0,-1));
    float p_02 = getPel(pel+vec2( 1,-1));
    float p_10 = getPel(pel+vec2(-1, 0));
    float p_11 = getPel(pel+vec2( 0, 0));
    float p_12 = getPel(pel+vec2( 1, 0));
    float p_20 = getPel(pel+vec2(-1, 1));
    float p_21 = getPel(pel+vec2( 0, 1));
    float p_22 = getPel(pel+vec2( 1, 1));
    
    
    //Gx
    float gx = -1*p_00 +
               -2*p_10 +
               -1*p_20 +
                1*p_02 +
                2*p_12 +
                1*p_22;
    //Gy
    float gy =  -1*p_00 +
                -2*p_01 +
                -1*p_02 +
                 1*p_20 +
                 2*p_21 +
                 1*p_22;
    
    return vec2(gx,gy);
}


void main(){

    //glFragcoord gives the center of the pixel, ie (0,0) -> (0.5, 0.5)
    vec2 coord = gl_FragCoord.xy - vec2(0.5,0.5);
    
    //Current source corner
    vec2 tex_uv = vec2(float(coord.x) / float(num_points),
                  0.0).xy;
    vec2 source_corner = texture(srcpts_texture_sampler, tex_uv).xy;

    
    //convert row index to an (x,y) location within the window with source pel at center
    int half_window_size = window_size/2;
    int row_number = int(coord.y);
    int window_x = int(mod(float(row_number), float(window_size))) - half_window_size;
    int window_y = row_number/window_size - half_window_size;
    vec2 current_window_location = source_corner + vec2(window_x, window_y);
    
    //Calculate the gradient at the current point
    vec2 gradient = getGradient(current_window_location);
    
    //dbg : check source image sampler
//    int pel = int(texture(srcimage_texture_sampler, vec2(float(100)/float(image_width),
  //                                                       float(100)/float(image_height))).r);
    
    
    a.xy = gradient.xy;
    w = 55;
}