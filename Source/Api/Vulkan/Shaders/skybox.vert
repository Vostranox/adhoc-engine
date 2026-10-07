#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform Sky {
	mat4 viewRotationProjection;
};

layout(location = 0) out vec3 outDirection;

const int corners[36] = int[](
	0, 2, 6, 0, 6, 4,
	1, 5, 7, 1, 7, 3,
	0, 4, 5, 0, 5, 1,
	2, 3, 7, 2, 7, 6,
	0, 1, 3, 0, 3, 2,
	4, 6, 7, 4, 7, 5);

void main() {
	int corner    = corners[gl_VertexIndex];
	vec3 position = vec3(corner & 1, (corner >> 1) & 1, (corner >> 2) & 1) * 2.0f - 1.0f;
	outDirection  = position;

	vec4 clip   = viewRotationProjection * vec4(position, 1.0f);
	gl_Position = clip.xyww;
}
