#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : enable

layout (set = 1, binding = 0) uniform Data {
	vec3 ambient;
	vec3 cameraPosition;
} ubo;

layout (set = 1, binding = 1) uniform DataPoint {
	DirectionalLight directionalLight;
};
