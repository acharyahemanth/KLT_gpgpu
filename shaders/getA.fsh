precision highp float;

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


//Returns the pixel value at the pel specified at the pyramid level specified
float getPel(vec2 pel){
    int shift = (1<<pyramid_level);
    vec2 pel_level0 = vec2(float(pel.x*float(shift)), float(pel.y*float(shift))).xy;
#ifdef GL_LINEAR_SAMPLING_AVAILABLE
    pel_level0 = pel_level0 + vec2(0.5,0.5);
    vec2 tex_uv = vec2(float(pel_level0.x) / float(image_width),
                       float(pel_level0.y) / float(image_height)).xy;
    return texture(srcimage_texture_sampler, tex_uv).x;
#else
    return blt(pel_level0.x, pel_level0.y, srcimage_texture_sampler);
#endif
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
    float gx = -1.0*p_00 +
               -2.0*p_10 +
               -1.0*p_20 +
                1.0*p_02 +
                2.0*p_12 +
                1.0*p_22;
    //Gy
    float gy =  -1.0*p_00 +
                -2.0*p_01 +
                -1.0*p_02 +
                 1.0*p_20 +
                 2.0*p_21 +
                 1.0*p_22;
    
    return vec2(gx,gy);
}


void main(){

    //glFragcoord gives the center of the pixel, ie (0,0) -> (0.5, 0.5)
    vec2 coord = gl_FragCoord.xy - vec2(0.5,0.5);
    
    //Current source corner
    vec2 tex_uv = vec2(float(gl_FragCoord.x) / float(num_points),
                  0.0).xy;
    vec2 source_corner = texture(srcpts_texture_sampler, tex_uv).xy;

    
    //convert row index to an (x,y) location within the window with source pel at center
    int half_window_size = int(float(window_size)/2.0);
    int row_number = int(coord.y);
    int window_x = int(mod(float(row_number)+EPS, float(window_size))) - half_window_size;
    int window_y = row_number/window_size - half_window_size;
    vec2 current_window_location = source_corner + vec2(window_x, window_y);
    
    //Calculate the gradient at the current point
    vec2 gradient = getGradient(current_window_location);
    
    //dbg : check source image sampler
//    int pel = int(texture(srcimage_texture_sampler, vec2(float(100)/float(image_width),
  //                                                       float(100)/float(image_height))).r);
    
    a.xy = gradient.xy;
}