#version 330 core

//Uniforms
uniform int num_points;
uniform int window_size;
uniform int image_width;
uniform int image_height;
//uniform int output_texture_width;
//uniform int output_texture_height;

//Texture samplers
uniform sampler2D srcimage_texture_sampler;
uniform sampler2D srcpts_texture_sampler;

//Outputs : a/b is a vec2 matrix of size window_size*window_size*#points
//To calculate a mat for a particular point, you need to deserialize a particular column of A
//a/w fills in one element in the a/b vec2 matrices
layout(location=0) out vec2 a;
layout(location=1) out float w;

void main(){
//	vec2 tex_uv;
//	tex_uv.x = float(gl_FragCoord.x) / float(output_texture_width);
//	tex_uv.y = float(gl_FragCoord.y) / float(output_texture_height);
//    float a = texture(a_texture_sampler, tex_uv).x;
//    float b = texture(b_texture_sampler, tex_uv).x;
//    float c = texture(c_texture_sampler, tex_uv).x;
    
    //dbg
    srcimage_texture_sampler;
    num_points;
    image_width;
    image_height;

    //glFragcoord gives the center of the pixel, ie (0,0) -> (0.5, 0.5)
    vec2 coord = gl_FragCoord.xy - vec2(0.5,0.5);
    
    //Current source corner
    vec2 tex_uv;
    tex_uv = vec2(float(coord.x) / float(num_points),
                  0.0).xy;
    vec2 source_corner = texture(srcpts_texture_sampler, tex_uv).xy;

    
    //convert row index to an (x,y) location within the window with source pel at center
    int half_window_size = window_size/2;
    int row_number = int(coord.y);
    int window_x = int(mod(float(row_number), float(window_size))) - half_window_size;
    int window_y = row_number/window_size - half_window_size;
    vec2 current_window_location = source_corner + vec2(window_x, window_y);
    
    a.xy = source_corner.yx;
    w = coord.y;
}