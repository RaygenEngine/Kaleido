#version 430 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec3 bitangent;
layout (location = 4) in vec2 text_coord0;
layout (location = 5) in vec2 text_coord1;

out Data
{ 
	vec3 world_frag_pos;

	vec3 tangent_frag_pos;
	vec3 tangent_view_pos;
	
	vec3 tangent_light_pos;	
	vec3 tangent_light_dir;
	
	vec2 text_coord[2];
} dataOut;

uniform struct PunctualLight
{
	vec3 world_pos;
	
	vec3 color;
	float intensity;
	
	float far;

	int atten_coef;
} punctual_light;

uniform mat4 mvp;
uniform mat4 m;

uniform mat3 normal_matrix; 

uniform vec3 view_pos;

void main()
{
    gl_Position = mvp * vec4(pos,1);
	dataOut.world_frag_pos = vec3(m * vec4(pos, 1.0));
	
	dataOut.text_coord[0] = text_coord0; 
	dataOut.text_coord[1] = text_coord1; 
	
	vec3 T = normalize(normal_matrix * tangent);
    vec3 B = normalize(normal_matrix * bitangent);
    vec3 N = normalize(normal_matrix * normal);

    mat3 TBN = transpose(mat3(T, B, N));
	
	dataOut.tangent_frag_pos = TBN * vec3(m * vec4(pos, 0.0));
	dataOut.tangent_view_pos  = TBN * view_pos;
	dataOut.tangent_light_pos = TBN * punctual_light.world_pos;
}