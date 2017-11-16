#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferObject {
	mat4 model;
	mat4 view;
} ubo;

layout(set = 1, binding = 0) uniform UniformCamera {
	mat4 proj;
	mat4 camPos;
} uc;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec2 outUV;

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
	vec4 pos = uc.camPos * ubo.model * vec4(inPosition, 1.0);
	gl_Position = uc.proj * pos; // * ubo.view * pos;
	//gl_Position.y = -gl_Position.y;
	//gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;

	outUV.x = inUV.x;
	outUV.y = 1.0 - inUV.y;
}