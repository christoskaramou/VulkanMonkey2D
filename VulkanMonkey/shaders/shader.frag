#version 450
#extension GL_ARB_separate_shader_objects : enable

#define lightCount 10

struct UniformLight{
	vec4 color;
	vec2 position;
	float radius;
	float on;
};
layout(push_constant) uniform Ambient {
	vec4 color;
} ambient;

layout(set = 0, binding = 1) uniform sampler2D texSampler;

layout(set = 2, binding = 0) uniform UniformLights {
	UniformLight pointLight[lightCount];
} light;

layout(location = 0) in vec2 inUV;
layout(location = 1) in vec4 inPos;

layout(location = 0) out vec4 outColor;

void main() {
	
	float distance = length(light.pointLight[0].position - inPos.xy) / light.pointLight[0].radius;

	float factor = 1/(distance*distance);

	outColor = texture(texSampler, inUV) * vec4(ambient.color.xyz, 1.0f) * ambient.color.w;
	outColor.w *= clamp(factor, 0.01, 1.0);
}