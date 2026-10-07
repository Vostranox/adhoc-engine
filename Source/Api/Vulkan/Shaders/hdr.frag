#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : enable

layout (binding = 1) uniform sampler2D hdrBuffer;
layout (binding = 2) uniform sampler2D bloomBuffer;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

layout(push_constant) uniform Tonemap {
     float bloomStrength;
     float exposure;
};

void main() {
	const float gamma = 2.2;
    vec3 hdrColor = clamp(texture(hdrBuffer, inUV).rgb, vec3(0.0), vec3(65504.0));
    vec3 bloomColor = texture(bloomBuffer, inUV).rgb;
    vec3 temp = mix(hdrColor, bloomColor, bloomStrength);
    vec3 result = vec3(1.0) - exp(-temp * exposure);
    result = pow(result, vec3(1.0 / gamma));
    outFragColor = vec4(result, 1.0);
}