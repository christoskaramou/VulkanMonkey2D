#version 450
#extension GL_ARB_separate_shader_objects : enable

#define lightCount 10

layout(set = 0, binding = 0) uniform UniformBufferObject {
	mat4 model;
} sprite;

layout(set = 1, binding = 0) uniform UniformCamera {
	mat4 proj;
	mat4 camPos;
} camera;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec4 outPos;


out gl_PerVertex {
	vec4 gl_Position;
};

void main() {

	outUV.x = inUV.x;
	outUV.y = 1.0 - inUV.y;

	outPos = sprite.model * vec4(inPosition, 1.0);

	gl_Position = camera.proj * camera.camPos * sprite.model * vec4(inPosition, 1.0);
	
}