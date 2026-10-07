#extension GL_ARB_separate_shader_objects : enable

layout (binding = 0) uniform sampler2D level;
layout (binding = 1) uniform sampler2D smaller;

layout (push_constant) uniform Upsample {
	float radius;
};

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

void main() {
	vec2 size = vec2(textureSize(smaller, 0));
	float x   = radius * size.y / size.x;
	float y   = radius;

	vec3 a = texture(smaller, inUV + vec2(-x, -y)).rgb;
	vec3 b = texture(smaller, inUV + vec2(0.0f, -y)).rgb;
	vec3 c = texture(smaller, inUV + vec2(x, -y)).rgb;
	vec3 d = texture(smaller, inUV + vec2(-x, 0.0f)).rgb;
	vec3 e = texture(smaller, inUV).rgb;
	vec3 f = texture(smaller, inUV + vec2(x, 0.0f)).rgb;
	vec3 g = texture(smaller, inUV + vec2(-x, y)).rgb;
	vec3 h = texture(smaller, inUV + vec2(0.0f, y)).rgb;
	vec3 i = texture(smaller, inUV + vec2(x, y)).rgb;

	vec3 upsampled = (4.0f * e + 2.0f * (b + d + f + h) + (a + c + g + i)) / 16.0f;
	outFragColor   = vec4(min(texture(level, inUV).rgb + upsampled, vec3(65504.0f)), 1.0f);
}
