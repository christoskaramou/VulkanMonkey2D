#pragma once
#include "glm_.h"
#include "BufferInfo.h"
#include <vector>
#define MAX_POINT_LIGHTS 20

namespace vm {
	//Spot,			// cone shape
	//Point,			// from point to all direction
	//Area,			// directional light rays from within a whole shape
	//Directional,	// parallel light rays in a single direction out into infinity
	//Volume,			// illuminates objects within its volume
	//Ambient			// light all objects

	// blocks of uniform memory that are passed to shaders must be usually mulitples of 16 bytes
	struct UniformLightObject {
		glm::vec4 color;		// 16 bytes
		glm::vec2 position;		// 8
		float radius;			// 4
		float on;				// 4
	};

	class PointLight 
	{
		friend class Renderer;
	public:
		static std::vector<PointLight*>		lightPool;
		static vk::DescriptorSet			descriptorSet;
		
		PointLight();
		~PointLight();

		void update();

		glm::vec2 getPos() const;
		void setPos(glm::vec2 position);
		float getLightAlpha() const;
		void setLightAlpha(float colorAlpha);
		void turnOn();
		void turnOff();
		void attachTo(glm::mat4 &attachMat4);
		void detach();
		void setRadius(float radius);
		float getRadius() const;

	private:
		unsigned int			pointLightID;
		glm::mat4				*attachedMat;
		void					*_uniformMemory; // pointer to uniform data memory
		BufferInfo				uloBuffInfo;
		UniformLightObject		ulo{};

		void setPointerToUniformBufferMem(vk::DeviceMemory &pointLightUniformBuffMem);

		static void createDescriptorSet(const vk::DescriptorPool &descriptorPool);
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

