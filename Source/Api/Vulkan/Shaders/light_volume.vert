#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : enable

#include "pbr_data.glsl"

layout(location = 0) in vec3 inPosition;

layout(set = 0, binding = 0) uniform Camera {
	mat4 projView;
};

layout(location = 0) flat out uint outLight;

const float rangeScale = 1.05f;

void main() {
	Light light = lights[gl_InstanceIndex];
	outLight    = uint(gl_InstanceIndex);
	gl_Position = projView * vec4(light.position + inPosition * light.range * rangeScale, 1.0f);
}
