#version 450

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 position;

uniform float scale;
uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

out vec3 position_vs;
out vec3 normal_vs;

void main() {
	mat4 mvp_matrix = projection_matrix*view_matrix*model_matrix;
    gl_Position = mvp_matrix*vec4(position + scale*vertex, 1.0);
	
	const mat4 normal_matrix = transpose(inverse(view_matrix*model_matrix));
   	
	vec4 pos1 = view_matrix*model_matrix*vec4(position + scale*vertex, 1.0);
	position_vs = pos1.xyz/pos1.w;
    normal_vs = vec3(normal_matrix*vec4(normal, 0));
}


