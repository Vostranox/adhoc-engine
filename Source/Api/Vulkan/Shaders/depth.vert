#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;

layout(set = 0, binding = 0) uniform UBO {
	mat4 projView;
};

layout(push_constant) uniform Model {
	mat4 model;
};

out gl_PerVertex {
	invariant vec4 gl_Position;
};

void main() {
	vec3 worldPosition = vec3(model * vec4(inPosition, 1.0f));
	gl_Position        = projView * vec4(worldPosition, 1.0f);
}
