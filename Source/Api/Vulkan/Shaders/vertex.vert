#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : enable

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;

layout (location = 0) out vec3 outColor;

layout (set = 0, binding = 0) uniform Ubo {
	mat4 model;
	mat4 view;
	mat4 projection;
};

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
	outColor    = inColor;
	gl_Position = projection * view * model * vec4(inPosition, 1.0f);
}
