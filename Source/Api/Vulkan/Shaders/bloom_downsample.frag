#extension GL_ARB_separate_shader_objects : enable

layout (binding = 0) uniform sampler2D source;

layout (push_constant) uniform Downsample {
	int karisAverage;
};

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

vec3 Fetch(vec2 uv) {
	return clamp(texture(source, uv).rgb, vec3(0.0f), vec3(65504.0f));
}

float KarisWeight(vec3 color) {
	float luma = dot(pow(color, vec3(1.0f / 2.2f)), vec3(0.299f, 0.587f, 0.114f));
	return 1.0f / (1.0f + luma * 0.25f);
}

void main() {
	vec2 texel = 1.0f / vec2(textureSize(source, 0));
	float x    = texel.x;
	float y    = texel.y;

	vec3 a = Fetch(inUV + vec2(-2.0f * x, -2.0f * y));
	vec3 b = Fetch(inUV + vec2(0.0f, -2.0f * y));
	vec3 c = Fetch(inUV + vec2(2.0f * x, -2.0f * y));
	vec3 d = Fetch(inUV + vec2(-2.0f * x, 0.0f));
	vec3 e = Fetch(inUV);
	vec3 f = Fetch(inUV + vec2(2.0f * x, 0.0f));
	vec3 g = Fetch(inUV + vec2(-2.0f * x, 2.0f * y));
	vec3 h = Fetch(inUV + vec2(0.0f, 2.0f * y));
	vec3 i = Fetch(inUV + vec2(2.0f * x, 2.0f * y));
	vec3 j = Fetch(inUV + vec2(-x, -y));
	vec3 k = Fetch(inUV + vec2(x, -y));
	vec3 l = Fetch(inUV + vec2(-x, y));
	vec3 m = Fetch(inUV + vec2(x, y));

	vec3 boxes[5]          = vec3[](0.25f * (a + b + d + e), 0.25f * (b + c + e + f), 0.25f * (d + e + g + h), 0.25f * (e + f + h + i), 0.25f * (j + k + l + m));
	const float weights[5] = float[](0.125f, 0.125f, 0.125f, 0.125f, 0.5f);

	vec3 color        = vec3(0.0f);
	float weightTotal = 0.0f;
	for (int n = 0; n < 5; ++n) {
		float weight = karisAverage != 0 ? weights[n] * KarisWeight(boxes[n]) : weights[n];
		color       += weight * boxes[n];
		weightTotal += weight;
	}
	outFragColor = vec4(color / weightTotal, 1.0f);
}
