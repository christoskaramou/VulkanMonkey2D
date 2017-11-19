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

layout(push_constant) uniform Ambient {
	vec4 color;
} ambient;

struct UniformLight{
	float radius;
	vec2 position;
	vec4 color;
	float paddingDump;
};

layout(set = 2, binding = 0) uniform UniformLights {
		float radius;
		vec2 position;
		vec4 color;
} light;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec4 outColor;
layout(location = 2) out vec4 outAmbient;


out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
	vec4 pos = camera.proj * camera.camPos * sprite.model * vec4(inPosition, 1.0);
	gl_Position = pos;

	outUV.x = inUV.x;
	outUV.y = 1.0 - inUV.y;
	
	outAmbient = ambient.color;

	/*float magn = 1.0;
	for (int i = 0; i < lightCount; ++i)
	{
		if (light.pointLight[i].radius > 0.0) {
			float factor = 1.0 - min(distance(light.pointLight[i].position, pos.xy), light.pointLight[i].radius) / (light.pointLight[i].radius*light.pointLight[i].radius);
		}
	}*/

	outColor = vec4(inColor, 1.0);
}