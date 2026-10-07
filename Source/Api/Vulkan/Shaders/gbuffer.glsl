#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : enable

layout(set = 0, binding = 0) uniform Camera {
	mat4 projView;
	mat4 inverseProjView;
};

layout(set = 3, binding = 2) uniform sampler2D gAlbedoMetallic;
layout(set = 3, binding = 3) uniform sampler2D gNormalRoughness;
layout(set = 3, binding = 4) uniform sampler2D gDepth;
layout(set = 3, binding = 5) uniform sampler2D gVertexNormal;

struct Surface {
	vec3  position;
	vec3  normal;
	vec3  vertexNormal;
	vec3  albedo;
	float metallicness;
	float roughness;
};

bool ReadSurface(ivec2 pixel, out Surface surface) {
	float depth = texelFetch(gDepth, pixel, 0).r;
	if (depth >= 1.0f) {
		return false;
	}

	vec2 ndc              = (vec2(pixel) + 0.5f) / vec2(textureSize(gDepth, 0)) * 2.0f - 1.0f;
	vec4 position         = inverseProjView * vec4(ndc, depth, 1.0f);
	vec4 albedoMetallic   = texelFetch(gAlbedoMetallic, pixel, 0);
	vec4 normalRoughness  = texelFetch(gNormalRoughness, pixel, 0);
	surface.position      = position.xyz / position.w;
	surface.normal        = normalize(normalRoughness.xyz);
	surface.vertexNormal  = normalize(texelFetch(gVertexNormal, pixel, 0).xyz * 2.0f - 1.0f);
	surface.albedo        = albedoMetallic.rgb * albedoScale;
	surface.metallicness  = albedoMetallic.a;
	surface.roughness     = normalRoughness.a;
	return true;
}
