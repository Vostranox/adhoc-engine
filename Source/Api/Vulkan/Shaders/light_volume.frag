#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : enable

#include "pbr_data.glsl"
#include "pbr_functions.glsl"
#include "pbr_lighting.glsl"
#include "gbuffer.glsl"

layout(location = 0) flat in uint inLight;

layout(location = 0) out vec4 outFragColor;

void main() {
	Surface surface;
	if (!ReadSurface(ivec2(gl_FragCoord.xy), surface)) {
		discard;
	}

	vec3 V            = normalize(ubo.cameraPosition - surface.position);
	vec3 reflectivity = mix(vec3(0.04f), surface.albedo, surface.metallicness);
	vec3 reflectance  = EvaluateLocalLight(lights[inLight], surface.position, surface.normal, V, surface.albedo, reflectivity, surface.roughness, surface.metallicness);
	outFragColor = vec4(reflectance, 0.0f);
}
