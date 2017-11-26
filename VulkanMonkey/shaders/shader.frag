#version 450
#extension GL_ARB_separate_shader_objects : enable

#define lightCount 20

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
	
	float factor = 0.0;

	for(int i=0; i<lightCount; i++){
		if (light.pointLight[i].on == 1.0){
			float distance = length(light.pointLight[i].position - inPos.xy) / light.pointLight[i].radius;
			factor += clamp(1/(distance*distance), 0.0, 1.0) * light.pointLight[i].color.w;
		}
	}
	outColor = texture(texSampler, inUV);
	outColor = vec4(outColor.xyz + ambient.color.xyz, outColor.w);
	outColor.w *= clamp(ambient.color.w + factor, 0.0, 1.0);
}