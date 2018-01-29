#version 450

in vec3 position_vs;
in vec3 normal_vs;
in mat4 normal_matrix;

const vec3 la = vec3(0.3);
const vec3 ld = vec3(0.5);
const vec3 ls = vec3(0.1);


const vec3 ka = vec3(1.0, 1.0, 1.0);
const vec3 ks = vec3(1.0, 1.0, 1.0);
const float shininess = 1.0;

uniform mat4 model_matrix;
uniform mat4 view_matrix;

out vec4 frag_color;

void main() {
	vec3 frag_color_vs = vec3(0, 1, 1);
    vec3 normal1 = normalize(normal_vs);

    vec3 pos_light = -view_matrix[3].xyz;
    vec3 s = normalize(pos_light - position_vs);
    vec3 v = normalize(-position_vs);
    vec3 r = reflect(-s, normal1);

    float sDotN = max(dot(s, normal1), 0.0);
    vec3 ambient = la*ka;
    vec3 diffuse = ld*frag_color_vs*sDotN;
    vec3 specular = vec3(0.0);
    if( sDotN > 0.0 )
        specular = ls*ks*pow(max(dot(r,v), 0.0), shininess);

    frag_color = vec4( diffuse + ambient + specular, 1 );
}

