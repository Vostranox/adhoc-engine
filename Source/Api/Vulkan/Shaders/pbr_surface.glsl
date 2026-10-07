#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : enable

layout (set = 2, binding = 0) uniform sampler2D albedoMap;
layout (set = 2, binding = 1) uniform sampler2D normalMap;
layout (set = 2, binding = 2) uniform sampler2D heightMap;

mat3 TangentSpace(vec3 normal, vec4 tangent) {
	vec3 N = normalize(normal);
	vec3 T = normalize(tangent.xyz - dot(tangent.xyz, N) * N);
	vec3 B = cross(N, T) * tangent.w;
	return mat3(T, B, N);
}

vec3 SurfaceNormal(mat3 tangentSpace, vec4 texel) {
	vec3 normal = texel.xyz * 2.0f - 1.0f;
	return normalize(tangentSpace * normal);
}

vec2 ParallaxOcclusion(vec2 uv, vec2 dx, vec2 dy, vec3 V, float heightScale) {
	const float minLayers = 8.0f;
	const float maxLayers = 64.0f;

	float layers     = mix(maxLayers, minLayers, abs(V.z));
	float layerDepth = 1.0f / layers;
	vec2  shift      = V.xy / max(V.z, 0.05f) * heightScale / layers;

	float depth    = 0.0f;
	float mapDepth = 1.0f - textureGrad(heightMap, uv, dx, dy).r;
	for (int i = 0; i <= int(maxLayers) && depth < mapDepth; ++i) {
		uv      -= shift;
		mapDepth = 1.0f - textureGrad(heightMap, uv, dx, dy).r;
		depth   += layerDepth;
	}

	vec2  previous = uv + shift;
	float after    = mapDepth - depth;
	float before   = 1.0f - textureGrad(heightMap, previous, dx, dy).r - depth + layerDepth;
	return mix(uv, previous, after / (after - before));
}
