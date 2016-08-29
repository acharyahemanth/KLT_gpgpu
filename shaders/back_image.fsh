precision highp float;

//Uniforms
uniform int image_width;
uniform int image_height;

//Texture samplers
uniform sampler2D srcimage_texture_sampler;

//Output to display buffer
out vec3 frag_color;

void main(){
    
    //glFragcoord gives the center of the pixel, ie (0,0) -> (0.5, 0.5)
    vec2 coord = gl_FragCoord.xy - vec2(0.5,0.5);
    
    //Current source corner
    vec2 tex_uv = vec2(float(coord.x) / float(image_width),
                       float(coord.y) / float(image_height)).xy;
    frag_color = texture(srcimage_texture_sampler, tex_uv).xyz;
}