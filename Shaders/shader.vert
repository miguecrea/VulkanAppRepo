
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

layout(location = 0) out vec3 fragCol;
layout(location = 1) out vec2 fragTex;
layout(location = 2) out vec3 vViewDir;
layout(location = 3) out vec3 vLightDir;
layout(location = 4) out vec3 vNormal;

void main() {
	gl_Position = uboViewProjection.projection * uboViewProjection.view * pushModel.model * vec4(pos, 1.0);

	vec3 eyePos = vec3(5, 0, 0);
	vec3 dirLightDir = normalize(vec3(1, 1, 1));

	fragCol = normal;
	fragTex = tex;
	vViewDir = normalize(eyePos - vec3(pushModel.model * vec4(pos, 1.0)));
	vLightDir = dirLightDir;
	vNormal = normal;
}