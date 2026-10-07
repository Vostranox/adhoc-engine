#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : enable

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec4 inColor;

layout (push_constant) uniform PushConstants {
	vec2 scale;
	vec2 translate;
};

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec4 outColor;

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
	outUV       = inUV;
	outColor    = inColor;
	gl_Position = vec4(inPos * scale + translate, 0.0f, 1.0f);
}
