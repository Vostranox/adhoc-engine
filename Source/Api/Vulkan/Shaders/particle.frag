#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 inCorner;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 outFragColor;

void main() {
	float disc   = 1.0f - smoothstep(0.0f, 1.0f, length(inCorner));
	outFragColor = vec4(inColor.rgb, inColor.a * disc);
}
