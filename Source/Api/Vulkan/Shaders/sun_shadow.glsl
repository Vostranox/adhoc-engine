#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : enable
#extension GL_EXT_texture_shadow_lod      : enable

const int MAX_SHADOW_CASCADES = 4;
const int SHADOW_TAPS         = 16;
const float GOLDEN_ANGLE      = 2.39996323f;
const float MIN_NORMAL_OFFSET = 0.05f;

layout(set = 0, binding = 1) uniform SunShadow {
	mat4  cascadeLightSpace[MAX_SHADOW_CASCADES];
	vec4  cascadeEnd;
	vec4  cascadeBlendStart;
	vec4  cascadeFilterRadius;
	vec4  cascadeNormalOffset;
	vec4  cameraForward;
	int   cascadeCount;
	int   showCascades;
} sunShadow;

layout(set = 1, binding = 2) uniform sampler2DArrayShadow sunShadowMap;

vec2 VogelDiskTap(int i) {
	float radius = sqrt((float(i) + 0.5f) / float(SHADOW_TAPS));
	float angle  = float(i) * GOLDEN_ANGLE;
	return radius * vec2(cos(angle), sin(angle));
}

float InterleavedGradientNoise(vec2 pixel) {
	return fract(52.9829189f * fract(dot(pixel, vec2(0.06711056f, 0.00583715f))));
}

float SampleCascade(int c, vec3 position, mat2 rotation) {
	vec4 shadowCoords = sunShadow.cascadeLightSpace[c] * vec4(position, 1.0f);
	float lit = 0.0f;
	for (int i = 0; i < SHADOW_TAPS; ++i) {
		vec2 uv = shadowCoords.xy + rotation * VogelDiskTap(i) * sunShadow.cascadeFilterRadius[c];
		lit += textureLod(sunShadowMap, vec4(uv, float(c), shadowCoords.z), 0.0f);
	}
	return lit / float(SHADOW_TAPS);
}

float SunVisibility(vec3 position, vec3 normal, vec3 L, vec2 pixel) {
	float depth = dot(position - ubo.cameraPosition, sunShadow.cameraForward.xyz);
	int last    = sunShadow.cascadeCount - 1;
	if (last < 0 || depth > sunShadow.cascadeEnd[last]) {
		return 1.0f;
	}

	int c = 0;
	while (c < last && depth > sunShadow.cascadeEnd[c]) {
		++c;
	}

	float NdotL   = clamp(dot(normal, L), 0.0f, 1.0f);
	vec3 offset   = normal * max(sqrt(1.0f - NdotL * NdotL), MIN_NORMAL_OFFSET);
	float angle   = 6.28318531f * InterleavedGradientNoise(pixel);
	mat2 rotation = mat2(cos(angle), sin(angle), -sin(angle), cos(angle));
	float lit     = SampleCascade(c, position + offset * sunShadow.cascadeNormalOffset[c], rotation);

	float blend = (depth - sunShadow.cascadeBlendStart[c]) / (sunShadow.cascadeEnd[c] - sunShadow.cascadeBlendStart[c]);
	if (blend > 0.0f) {
		float next = c < last ? SampleCascade(c + 1, position + offset * sunShadow.cascadeNormalOffset[c + 1], rotation) : 1.0f;
		lit        = mix(lit, next, blend);
	}
	return lit;
}

vec3 SunCascadeTint(vec3 position) {
	if (sunShadow.showCascades == 0) {
		return vec3(1.0f);
	}
	const vec3 colors[MAX_SHADOW_CASCADES] = vec3[](vec3(1.0f, 0.3f, 0.3f), vec3(0.3f, 1.0f, 0.3f), vec3(0.3f, 0.3f, 1.0f), vec3(1.0f, 1.0f, 0.3f));
	float depth = dot(position - ubo.cameraPosition, sunShadow.cameraForward.xyz);
	for (int c = 0; c < sunShadow.cascadeCount; ++c) {
		if (depth <= sunShadow.cascadeEnd[c]) {
			return colors[c];
		}
	}
	return vec3(1.0f);
}
