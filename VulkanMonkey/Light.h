#pragma once
#include "glm_.h"
#include "BufferInfo.h"
#include <vector>
#define MAX_POINT_LIGHTS 10

namespace vm {
	//Spot,			// cone shape
	//Point,			// from point to all direction
	//Area,			// directional light rays from within a whole shape
	//Directional,	// parallel light rays in a single direction out into infinity
	//Volume,			// illuminates objects within its volume
	//Ambient			// light all objects
	struct UniformLightObject {
		glm::vec4 color;		// 16 read block
		glm::vec2 position;		// 8 +
		float radius;			// 4 +
		float on;
	};

	class PointLight 
	{
		friend class Renderer;
	public:
		static std::vector<PointLight*>		lightPool;
		static vk::DescriptorSet			descriptorSet;
		
		PointLight();
		~PointLight();

		BufferInfo					uloBuffInfo;
		UniformLightObject			ulo{};

	private:
		unsigned int	pointLightID;
		glm::mat4		*attachedMat;
		void			*_uniformMemory; // pointer to uniform data memory

		void setPointerToUniformBufferMem(vk::DeviceMemory &pointLightUniformBuffMem);

		static void createDescriptorSet(const vk::DescriptorPool &descriptorPool);

	public:
		void update();

		glm::vec2 getPos();
		void setPos(glm::vec2 position);
		glm::vec4 getColor();
		void setColor(glm::vec4 color);
		void turnOn();
		void turnOff();
		void attachTo(glm::mat4 &attachMat4);
		void setRadius(float radius);
		float getRadius();
	};

	class AmbientLight
	{
	public:
		static glm::vec4 color;

	private:

		AmbientLight();
		~AmbientLight();
	};

}

