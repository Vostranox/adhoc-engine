#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;

layout(push_constant) uniform Outline {
	mat4 transform;
};

void main() {
	gl_Position = transform * vec4(inPosition, 1.0f);
}
