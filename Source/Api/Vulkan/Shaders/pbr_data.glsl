#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : enable

const float PI = 3.14159265359;

const float albedoScale = 0.25;

const int LIGHT_POINT = 0;
const int LIGHT_SPOT  = 1;

struct Material {
	float roughness;
	float metallicness;
	float transparency;
	float heightScale;
	vec3  albedo;
	float emissive;
	float ambientOcclusion;
};

struct DirectionalLight {
	vec3  direction;
	vec3  color;
	float intensity;
	int   castsShadow;
	int   samplerId;
};

struct Light {
	vec3  position;
	float range;
	vec3  color;
	float intensity;
	vec3  direction;
	int   type;
	float cosInner;
	float cosOuter;
};

layout(std430, set = 1, binding = 3) readonly buffer Lights {
	uint  lightCount;
	Light lights[];
};
