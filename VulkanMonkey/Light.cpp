#include "Light.h"
#include "ResourceManager.h"
#include "ErrorAndLog.h"

namespace vm {
	std::vector<PointLight*>	PointLight::lightPool(MAX_POINT_LIGHTS);
	vk::DescriptorSet			PointLight::descriptorSet;

	PointLight::PointLight()
	{
		ResourceManager &rm = ResourceManager::getInstance();
		static unsigned int pointLightNumber = 0;		// every pointLight has a unique ID
		pointLightID = pointLightNumber++;
		if (pointLightNumber > MAX_POINT_LIGHTS) {
			--pointLightNumber;
			LOG("MAX_POINT_LIGHTS number aleady exceeded! Define a bigger value with cautious.\n");
			return;
		}
		
		uloBuffInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer;
		uloBuffInfo.size = sizeof(UniformLightObject);
		if (uloBuffInfo.size < rm.getGpuProperties().limits.minUniformBufferOffsetAlignment)
			uloBuffInfo.size = rm.getGpuProperties().limits.minUniformBufferOffsetAlignment;
		uloBuffInfo.offset = pointLightID * uloBuffInfo.size;

		ulo.position = glm::vec2(0.f);
		ulo.color = glm::vec4(1.f);
		ulo.radius = 0.f;
		ulo.on = 0.f;
		attachedMat = nullptr;
		_uniformMemory = nullptr;
		PointLight::lightPool[pointLightID] = this;
	}
	PointLight::~PointLight()
	{
	}
	glm::vec2 PointLight::getPos()
	{
		return ulo.position;
	}
	void PointLight::setPos(glm::vec2 position)
	{
		ulo.position = position;
	}
	glm::vec4 PointLight::getColor()
	{
		return ulo.color;
	}
	void PointLight::setColor(glm::vec4 color)
	{
		ulo.color = color;
	}
	void PointLight::turnOn()
	{
		ulo.on = 1.f;
	}
	void PointLight::turnOff()
	{
		ulo.on = 0.f;
	}
	void PointLight::attachTo(glm::mat4 & attachMat4)
	{
		attachedMat = &attachMat4;
	}
	void PointLight::setRadius(float radius)
	{
		ulo.radius = radius;
	}
	float PointLight::getRadius()
	{
		return ulo.radius;
	}

	void PointLight::update()
	{
		if (!_uniformMemory) {
			LOG("Uniform Buffer Memory of Light: " << pointLightID << " is not mapped\n");
			return;
		}
		if (attachedMat) {
			glm::vec4 pos = *attachedMat * glm::vec4(1.f);
			ulo.position.x = pos.x;
			ulo.position.y = pos.y;
		}
		memcpy(_uniformMemory, &ulo, uloBuffInfo.size);
	}

	void PointLight::setPointerToUniformBufferMem(vk::DeviceMemory &pointLightUniformBuffMem)
	{
		ResourceManager &rm = ResourceManager::getInstance();
		// just get the pointer to the pointLightUniformBufferMem offset for this sprite
		rm.getDevice().mapMemory(pointLightUniformBuffMem, uloBuffInfo.offset, uloBuffInfo.size, vk::MemoryMapFlags(), &_uniformMemory);
		rm.getDevice().unmapMemory(pointLightUniformBuffMem);
	}
	void PointLight::createDescriptorSet(const vk::DescriptorPool &descriptorPool)
	{
		ResourceManager &rm = ResourceManager::getInstance();
		// POINTLIGHTS DESCRIPTORSET
		auto const plAllocateInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(descriptorPool)
			.setDescriptorSetCount(1)
			.setPSetLayouts(&ResourceManager::getInstance().pointLightsDescriptorSetLayout);
		rm.getDevice().allocateDescriptorSets(&plAllocateInfo, &PointLight::descriptorSet);
		vk::DeviceSize size = sizeof(UniformLightObject);
		if (size < rm.getGpuProperties().limits.minUniformBufferOffsetAlignment)
			size = rm.getGpuProperties().limits.minUniformBufferOffsetAlignment;
		auto const plWriteDset = vk::WriteDescriptorSet()
			.setDstSet(PointLight::descriptorSet)									//descriptor set
			.setDstBinding(0)											//binding number in shader
			.setDstArrayElement(0)										//start element in array
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)		//descriptor type
			.setDescriptorCount(1)										//descriptor count
			.setPBufferInfo(&vk::DescriptorBufferInfo()
				.setBuffer(rm.pointLightsUniformBuffer)					//buffer
				.setOffset(0)											//buffer offset
				.setRange(size * MAX_POINT_LIGHTS));							//buffer size	
		rm.getDevice().updateDescriptorSets(1, &plWriteDset, 0, nullptr);
	}

	// Ambient class
	glm::vec4 AmbientLight::color = glm::vec4(1.f);
	AmbientLight::AmbientLight() {}
	AmbientLight::~AmbientLight() {}
}
