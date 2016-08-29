precision highp float;

//Uniforms
uniform int num_points;
uniform int window_size;
uniform int image_width;
uniform int image_height;
uniform int pyramid_level;

//Texture samplers
uniform sampler2D srcimage_texture_sampler;
uniform sampler2D dstimage_texture_sampler;
uniform sampler2D srcpts_texture_sampler;
uniform sampler2D predpts_texture_sampler;

//Outputs : a/b is a vec2 matrix of size window_size*window_size*#points
//To calculate a mat for a particular point, you need to deserialize a particular column of b
//a/w fills in one element in the a/b vec2 matrices
layout(location=0) out float b;

float EPS = 1e-5;

//Returns the pixel value from source img at the pel specified at the pyramid level specified
float getSourcePel(vec2 pel){
    float shift = float(1<<pyramid_level);
    vec2 pel_level0 = vec2(pel.x*shift, pel.y*shift).xy;
    pel_level0 = pel_level0 + vec2(0.5,0.5);
    vec2 tex_uv = vec2(float(pel_level0.x) / float(image_width),
                       float(pel_level0.y) / float(image_height)).xy;
    return texture(srcimage_texture_sampler, tex_uv).x;
}

//Returns the pixel value from dest img at the pel specified at the pyramid level specified
float getDestPel(vec2 pel){
    float shift = float(1<<pyramid_level);
    vec2 pel_level0 = vec2(pel.x*shift, pel.y*shift).xy;
    pel_level0 = pel_level0 + vec2(0.5,0.5);
    vec2 tex_uv = vec2(float(pel_level0.x) / float(image_width),
                       float(pel_level0.y) / float(image_height)).xy;
    return texture(dstimage_texture_sampler, tex_uv).x;
}


//Function calculates the difference b/w source pel and dest pel from pyramid of images
float getDelta(vec2 source_pel, vec2 dest_pel){

    float p_src = getSourcePel(source_pel);
    float p_dst = getDestPel(dest_pel);
    
    return p_src-p_dst;
}


void main(){
    
    //glFragcoord gives the center of the pixel, ie (0,0) -> (0.5, 0.5)
    vec2 coord = gl_FragCoord.xy - vec2(0.5,0.5);
    
    //Current source and corresponding prediction corners
    vec2 tex_uv = vec2(float(coord.x) / float(num_points),
                       0.0).xy;
    vec2 source_corner = texture(srcpts_texture_sampler, tex_uv).xy;
    vec2 pred_corner = texture(predpts_texture_sampler, tex_uv).xy;
    
    
    //convert row index to an (x,y) location within the window with source pel at center
    int half_window_size = window_size/2;
    int row_number = int(coord.y);
    int window_x = int(mod(float(row_number)+EPS, float(window_size))) - half_window_size;
    int window_y = row_number/window_size - half_window_size;
    vec2 current_source_loc = source_corner + vec2(window_x, window_y);
    vec2 current_pred_loc = pred_corner + vec2(window_x, window_y);
    
    //Calculate the gradient at the current point
    float delta = getDelta(current_source_loc, current_pred_loc);
    
    //dbg : check source image sampler
    //    int pel = int(texture(srcimage_texture_sampler, vec2(float(100)/float(image_width),
    //                                                       float(100)/float(image_height))).r);
    
    b = delta;
}