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

//#define GL_LINEAR_SAMPLING_AVAILABLE

//A--B
//D--C
float blt(float x, float y, sampler2D sampler){
    float lu_x = float(int(x));
    float lu_y = float(int(y));
    float rd_x = float(int(x+1.0));
    float rd_y = float(int(y+1.0));
    float alpha_x = x-lu_x;
    float alpha_y = y-lu_y;

    lu_x += 0.5;
    lu_y += 0.5;
    rd_x += 0.5;
    rd_y += 0.5;


    //Get pels of corner of square
    vec2 tex_A = vec2(float(lu_x) / float(image_width),
                      float(lu_y) / float(image_height));
    vec2 tex_B = vec2(float(rd_x) / float(image_width),
                      float(lu_y) / float(image_height));
    vec2 tex_C = vec2(float(rd_x) / float(image_width),
                      float(rd_y) / float(image_height));
    vec2 tex_D = vec2(float(lu_x) / float(image_width),
                      float(rd_y) / float(image_height));
    float pel_A = texture(sampler, tex_A).x;
    float pel_B = texture(sampler, tex_B).x;
    float pel_C = texture(sampler, tex_C).x;
    float pel_D = texture(sampler, tex_D).x;


    float x_interp_lu = (1.0-alpha_x)*pel_A + alpha_x*pel_B;
    float x_interp_rd = (1.0-alpha_x)*pel_D + alpha_x*pel_C;
    float final_interp_val = (1.0-alpha_y)*x_interp_lu + alpha_y*x_interp_rd;
    
    return final_interp_val;
}

//Returns the pixel value from source img at the pel specified at the pyramid level specified
float getSourcePel(vec2 pel){
    float shift = float(1<<pyramid_level);
    vec2 pel_level0 = vec2(pel.x*shift, pel.y*shift).xy;
#ifdef GL_LINEAR_SAMPLING_AVAILABLE
    pel_level0 = pel_level0 + vec2(0.5,0.5);
    vec2 tex_uv = vec2(float(pel_level0.x) / float(image_width),
                       float(pel_level0.y) / float(image_height)).xy;
    return texture(srcimage_texture_sampler, tex_uv).x;
#else
    return blt(pel_level0.x, pel_level0.y, srcimage_texture_sampler);
#endif
}

//Returns the pixel value from dest img at the pel specified at the pyramid level specified
float getDestPel(vec2 pel){
    float shift = float(1<<pyramid_level);
    vec2 pel_level0 = vec2(pel.x*shift, pel.y*shift).xy;
#ifdef GL_LINEAR_SAMPLING_AVAILABLE
    pel_level0 = pel_level0 + vec2(0.5,0.5);
    vec2 tex_uv = vec2(float(pel_level0.x) / float(image_width),
                       float(pel_level0.y) / float(image_height)).xy;
    return texture(dstimage_texture_sampler, tex_uv).x;
#else
    return blt(pel_level0.x, pel_level0.y, dstimage_texture_sampler);
#endif
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
    vec2 tex_uv = vec2(float(gl_FragCoord.x) / float(num_points),
                       0.0).xy;
    vec2 source_corner = texture(srcpts_texture_sampler, tex_uv).xy;
    vec2 pred_corner = texture(predpts_texture_sampler, tex_uv).xy;
    
    
    //convert row index to an (x,y) location within the window with source pel at center
    int half_window_size = int(float(window_size)/2.0);
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