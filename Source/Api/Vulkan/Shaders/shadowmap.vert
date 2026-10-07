#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : enable

layout(location = 0) in vec3 inPosition;

layout(push_constant) uniform Transform {
   mat4 uTransform;
};

void main() {
   gl_Position = uTransform * vec4(inPosition, 1.0f);
}
