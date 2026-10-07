#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : enable

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormals;
layout (location = 2) in vec2 inTextureCoords;
layout (location = 3) in vec4 inTangent;

layout(location = 0) out vec3 outNormals;
layout(location = 1) out vec2 outTextureCoords;
layout(location = 2) out vec3 outWorldPosition;
layout(location = 3) out vec4 outTangent;

layout(set = 0, binding = 0) uniform UBO  {
	mat4 projView;
};

layout(push_constant) uniform Model {
     mat4 model;
};

out gl_PerVertex {
	invariant vec4 gl_Position;
};

void main() {
	outNormals       = mat3(transpose(inverse(model))) * inNormals;
	outWorldPosition = vec3(model * vec4(inPosition, 1.0f));
    outTextureCoords = inTextureCoords;
	outTangent       = vec4(mat3(model) * inTangent.xyz, inTangent.w);
	gl_Position      = projView * vec4(outWorldPosition, 1.0f);
}
