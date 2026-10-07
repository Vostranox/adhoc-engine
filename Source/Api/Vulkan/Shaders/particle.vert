#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 inPositionSize;
layout(location = 1) in vec4 inColor;

layout(push_constant) uniform Camera {
	mat4 viewProjection;
	vec4 right;
	vec4 up;
};

layout(location = 0) out vec2 outCorner;
layout(location = 1) out vec4 outColor;

const vec2 corners[6] = vec2[](vec2(-1.0f, -1.0f), vec2(1.0f, -1.0f), vec2(1.0f, 1.0f), vec2(-1.0f, -1.0f), vec2(1.0f, 1.0f), vec2(-1.0f, 1.0f));

void main() {
	vec2 corner   = corners[gl_VertexIndex];
	vec3 position = inPositionSize.xyz + (corner.x * right.xyz + corner.y * up.xyz) * inPositionSize.w;
	outCorner     = corner;
	outColor      = inColor;
	gl_Position   = viewProjection * vec4(position, 1.0f);
}
