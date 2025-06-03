#version 450 		// Use GLSL 4.5

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 col;
layout(location = 2) in vec2 tex;
layout(location = 3) in vec3 normal;

layout(set = 0, binding = 0) uniform UboViewProjection {
	mat4 projection;
	mat4 view;
} uboViewProjection;

// NOT IN USE, LEFT FOR REFERENCE
layout(set = 0, binding = 1) uniform UboModel {
	mat4 model;
} uboModel;

layout(push_constant) uniform PushModel {
	mat4 model;
} pushModel;


out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() {
	gl_Position = uboViewProjection.projection * uboViewProjection.view * pushModel.model * vec4(pos, 1.0);
}