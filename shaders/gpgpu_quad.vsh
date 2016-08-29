//inputs
in vec3 vert;


void main(){
	//Pass on the quad vertices as they are---
	gl_Position = vec4(vert,1.0);
}