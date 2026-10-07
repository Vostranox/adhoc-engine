#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : enable

#include "pbr_data.glsl"
#include "pbr_functions.glsl"
#include "pbr_lighting.glsl"
#include "gbuffer.glsl"
#include "sun_shadow.glsl"

layout(location = 0) out vec4 outFragColor;

void main() {
	Surface surface;
	if (!ReadSurface(ivec2(gl_FragCoord.xy), surface)) {
		discard;
	}

	vec3 V            = normalize(ubo.cameraPosition - surface.position);
	vec3 reflectivity = mix(vec3(0.04f), surface.albedo, surface.metallicness);
	vec3 L            = normalize(directionalLight.direction);

	vec3 sunRadiance = directionalLight.color * directionalLight.intensity * SunCascadeTint(surface.position);
	vec3 reflectance = SunVisibility(surface.position, surface.vertexNormal, L, gl_FragCoord.xy) * EvaluateLight(surface.normal, V, L, sunRadiance, surface.albedo, reflectivity, surface.roughness, surface.metallicness);
	outFragColor = vec4(reflectance, 0.0f);
}
