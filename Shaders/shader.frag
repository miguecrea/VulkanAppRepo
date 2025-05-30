#version 450

#define PI 3.14159265

layout(location = 0) in vec3 fragCol;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 vViewDir;
layout(location = 3) in vec3 vLightDir;
layout(location = 4) in vec3 vNormal;

layout(set = 1, binding = 0) uniform sampler2D albedoMap;
layout(set = 1, binding = 1) uniform sampler2D roughnessMap;
layout(set = 1, binding = 2) uniform sampler2D normalMap;
layout(set = 1, binding = 3) uniform sampler2D emissiveMap;

layout(location = 0) out vec4 outColor; 	// Final output	 color (must also have location)

vec3 CookTorrance(vec3 materialDiffuseColor,
	float roughness,
	vec3 materialSpecularColor,
	vec3 normal,
	vec3 lightDir,
	vec3 viewDir,
	vec3 lightColor)
{
	float F0 = 0.8;
    float k = 0.2;

	float NdotL = max(0, dot(normal, lightDir));
	float Rs = 0.0;
	if (NdotL > 0) 
	{
		vec3 H = normalize(lightDir + viewDir);
		float NdotH = max(0, dot(normal, H));
		float NdotV = max(0, dot(normal, viewDir));
		float VdotH = max(0, dot(lightDir, H));

		// Fresnel reflectance
		float F = pow(1.0 - VdotH, 5.0);
		F *= (1.0 - F0);
		F += F0;

		// Microfacet distribution by Beckmann
		float m_squared = roughness * roughness;
		float r1 = 1.0 / (4.0 * m_squared * pow(NdotH, 4.0));
		float r2 = (NdotH * NdotH - 1.0) / (m_squared * NdotH * NdotH);
		float D = r1 * exp(r2);

		// Geometric shadowing
		float two_NdotH = 2.0 * NdotH;
		float g1 = (two_NdotH * NdotV) / VdotH;
		float g2 = (two_NdotH * NdotL) / VdotH;
		float G = min(1.0, min(g1, g2));

		Rs = max(0, (F * D * G) / (PI * NdotL * NdotV));
	}
	return materialDiffuseColor * lightColor * NdotL + lightColor * materialSpecularColor * Rs;
}

void main() {
	vec3 specularColor = vec3(1, 1, 1);
	vec3 lightColor = vec3(1, 1, 1);

	vec3 albedo = texture(albedoMap, texCoord).rgb;
	float roughness = texture(roughnessMap, texCoord).g;
	vec3 normal = normalize(texture(normalMap, texCoord).rgb);
	vec3 emissive = texture(emissiveMap, texCoord).rgb;

	outColor = vec4(
		CookTorrance(albedo,
			roughness,
			specularColor,
			vNormal,
			vLightDir,
			vViewDir,
			lightColor) + albedo * 0.2,
		1);
}