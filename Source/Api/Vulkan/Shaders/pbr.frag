#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : enable
//#extension GL_EXT_debug_printf : enable

#include "pbr_data.glsl"
#include "pbr_functions.glsl"
#include "pbr_surface.glsl"
#include "pbr_lighting.glsl"
#ifndef GBUFFER
#include "sun_shadow.glsl"
#endif

 layout(location = 0) in vec3 inNormals;
 layout(location = 1) in vec2 inTextureCoords;
 layout(location = 2) in vec3 inWorldPosition;
 layout(location = 3) in vec4 inTangent;

 layout(location = 0) out vec4 outFragColor;
 layout(location = 1) out uvec2 outEntityId;

#ifdef GBUFFER
layout(location = 2) out vec4 outAlbedoMetallic;
layout(location = 3) out vec4 outNormalRoughness;
layout(location = 4) out float outDepth;
layout(location = 5) out vec4 outVertexNormal;
#endif

#ifdef LIGHT_TILES
#include "light_tiles.glsl"

layout(std430, set = 3, binding = 0) readonly buffer LightTiles {
	uint tiles[];
};
#endif

 layout(push_constant) uniform Materials {
	layout(offset = 64) Material material;
	layout(offset = 112) uvec2 entityId;
#ifdef LIGHT_TILES
	layout(offset = 120) uint tilesPerRow;
#endif
 };

 void main() {
 	mat3 tangentSpace = TangentSpace(inNormals, inTangent);
	vec3 V            = normalize(ubo.cameraPosition - inWorldPosition);

	vec2 uv = inTextureCoords;
	vec4 normalTexel;
	vec4 albedoTexel;
	if (material.heightScale != 0.0f) {
		vec2 dx     = dFdx(uv);
		vec2 dy     = dFdy(uv);
		uv          = ParallaxOcclusion(uv, dx, dy, transpose(tangentSpace) * V, material.heightScale);
		normalTexel = textureGrad(normalMap, uv, dx, dy);
		albedoTexel = textureGrad(albedoMap, uv, dx, dy);
	} else {
		normalTexel = texture(normalMap, uv);
		albedoTexel = texture(albedoMap, uv);
	}
	vec3 N = SurfaceNormal(tangentSpace, normalTexel);

	vec3 albedo       = material.albedo * albedoScale * albedoTexel.rgb;
#ifdef GBUFFER
	outFragColor       = vec4(ubo.ambient * material.ambientOcclusion * albedo + material.albedo * material.emissive, 1.0f);
	outEntityId        = entityId;
	outAlbedoMetallic  = vec4(material.albedo * albedoTexel.rgb, material.metallicness);
	outNormalRoughness = vec4(N, material.roughness);
	outDepth           = gl_FragCoord.z;
	outVertexNormal    = vec4(tangentSpace[2] * 0.5f + 0.5f, 0.0f);
#else
	vec3 reflectivity = mix(vec3(0.04f), albedo, material.metallicness);

	vec3 L           = normalize(directionalLight.direction);
	vec3 sunRadiance = directionalLight.color * directionalLight.intensity * SunCascadeTint(inWorldPosition);
	vec3 reflectance = SunVisibility(inWorldPosition, tangentSpace[2], L, gl_FragCoord.xy) * EvaluateLight(N, V, L, sunRadiance, albedo, reflectivity, material.roughness, material.metallicness);
#ifdef LIGHT_TILES
	uint first = FirstOfTile(uvec2(gl_FragCoord.xy) / TILE_SIZE, tilesPerRow);
	for (uint i = 1u; i <= tiles[first]; ++i) {
		reflectance += EvaluateLocalLight(lights[tiles[first + i]], inWorldPosition, N, V, albedo, reflectivity, material.roughness, material.metallicness);
	}
#else
	for (uint i = 0u; i < lightCount; ++i) {
		reflectance += EvaluateLocalLight(lights[i], inWorldPosition, N, V, albedo, reflectivity, material.roughness, material.metallicness);
	}
#endif

	vec3 emitted = material.albedo * material.emissive;

	vec3 ambient = ubo.ambient * material.ambientOcclusion * albedo;
	vec3 color   = ambient + reflectance + emitted;

	outFragColor = vec4(color, material.transparency * albedoTexel.a);
	outEntityId  = entityId;
#endif
}
