#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform samplerCube sky;

layout(push_constant) uniform Sky {
	layout(offset = 64) float intensity;
};

layout(location = 0) in vec3 inDirection;

layout(location = 0) out vec4 outFragColor;
layout(location = 1) out uvec2 outEntityId;

void main() {
	outFragColor = vec4(texture(sky, inDirection).rgb * intensity, 1.0f);
	outEntityId  = uvec2(0u);
}
