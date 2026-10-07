#extension GL_ARB_separate_shader_objects : enable

layout (binding = 0) uniform sampler2D image;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

void main() {
	outFragColor = vec4(texture(image, inUV).rgb, 1.0);
}
