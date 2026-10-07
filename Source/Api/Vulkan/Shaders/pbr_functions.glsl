#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : enable

float DistributionGGX(float NdotH, float roughness) {
	float alpha  = roughness * roughness;
	float alpha2 = alpha * alpha;

	float denom = NdotH * NdotH * (alpha2 - 1.0f) + 1.0f;
	denom = PI * denom * denom;

	return alpha2 / max(denom, 0.0000001f);
}

float GeometrySmith(float NdotV, float NdotL, float roughness) {
	float r = roughness + 1.0f;
	float k = (r * r) / 8.0f;

	float ggx1 = NdotV / (NdotV * (1.0f - k) + k);
	float ggx2 = NdotL / (NdotL * (1.0f - k) + k);

	return ggx1 * ggx2;
}

vec3 FresnelSchlick(float NdotV, vec3 baseReflectivity) {
	return baseReflectivity + (1.0f - baseReflectivity) * pow(1.0f - NdotV, 5.0f);
}

vec3 EvaluateLight(vec3 N, vec3 V, vec3 L, vec3 radiance, vec3 albedo, vec3 F0, float roughness, float metallicness) {
	vec3 H = normalize(V + L);

	float NdotV = max(dot(N, V), 0.0000001f);
	float NdotL = max(dot(N, L), 0.0000001f);
	float HdotV = max(dot(H, V), 0.0f);
	float NdotH = max(dot(N, H), 0.0f);

	float D = DistributionGGX(NdotH, roughness);
	float G = GeometrySmith(NdotV, NdotL, roughness);
	vec3  F = FresnelSchlick(HdotV, F0);

	vec3 diffuse = vec3(1.0f) - F;
	diffuse     *= 1.0f - metallicness;

	vec3 specular = D * G * F;
	specular /= 4.0f * NdotV * NdotL;

	return ((diffuse * albedo / PI + specular) * radiance * NdotL);
}

float DistanceAttenuation(float distance, float range) {
	float ratio  = distance / range;
	float window = clamp(1.0f - ratio * ratio * ratio * ratio, 0.0f, 1.0f);
	return window * window / (distance * distance + 1.0f);
}

float SpotFactor(float cosAngle, float cosInner, float cosOuter) {
	float t = clamp((cosAngle - cosOuter) / max(cosInner - cosOuter, 0.0001f), 0.0f, 1.0f);
	return t * t * (3.0f - 2.0f * t);
}

vec3 EvaluateLocalLight(Light light, vec3 worldPosition, vec3 N, vec3 V, vec3 albedo, vec3 F0, float roughness, float metallicness) {
	vec3  toLight  = light.position - worldPosition;
	float distance = length(toLight);
	vec3  L        = toLight / max(distance, 0.0001f);

	float falloff = DistanceAttenuation(distance, light.range);
	if (light.type == LIGHT_SPOT) {
		falloff *= SpotFactor(dot(-L, light.direction), light.cosInner, light.cosOuter);
	}
	if (falloff <= 0.0f) {
		return vec3(0.0f);
	}
	return EvaluateLight(N, V, L, light.color * light.intensity * falloff, albedo, F0, roughness, metallicness);
}
