#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec2 inUV;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec4 inAmbient;

layout(location = 0) out vec4 outColor;

void main() {
	outColor =	texture(texSampler, inUV) *
				vec4(inAmbient.xyz, 1.0f) *
				inAmbient.w;
}