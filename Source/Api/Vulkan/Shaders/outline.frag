#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform Outline {
	layout(offset = 64) vec4 color;
};

layout(location = 0) out vec4 outFragColor;

void main() {
	outFragColor = color;
}
