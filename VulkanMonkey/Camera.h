#pragma once
#include "glm_.h"
#include "Vulkan_.h"

namespace vm {
	struct UniformCameraBufferObject {
		glm::mat4 proj;
		glm::mat4 camPos;
	};

	struct Camera {
		friend class Renderer;
	private:
		void						*data;
		glm::mat4					startingCamPos;
		UniformCameraBufferObject	UCBO;
		bool						isUCBOmapped;
		float						zoom;
		vk::DeviceSize				bufferSize;
		vk::Buffer					uniformBuffer;
		vk::DeviceMemory			uniformBufferMem;
		vk::DescriptorSet			descriptorSet;
		glm::mat4					*attachedMat;
		Helper						helper;
	public:
		glm::vec3					position;
		glm::vec3					lookVector;
		glm::vec3					upVector;

	private:
		void createDescriptorSet(const vk::DescriptorPool &descriptorPool)
		{
			ResourceManager &rm = ResourceManager::getInstance();
			// CAMERA DESCRIPTORSET
			auto const camAllocateInfo = vk::DescriptorSetAllocateInfo()
				.setDescriptorPool(descriptorPool)
				.setDescriptorSetCount(1)
				.setPSetLayouts(&ResourceManager::getInstance().cameraDescriptorSetLayout);
			rm.getDevice().allocateDescriptorSets(&camAllocateInfo, &descriptorSet);
			auto const camWriteDset = vk::WriteDescriptorSet()
				.setDstSet(descriptorSet)							//descriptor set
				.setDstBinding(0)											//binding number in shader
				.setDstArrayElement(0)										//start element in array
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)		//descriptor type
				.setDescriptorCount(1)										//descriptor count
				.setPBufferInfo(&vk::DescriptorBufferInfo()
					.setBuffer(uniformBuffer)						//buffer
					.setOffset(0)											//buffer offset
					.setRange(bufferSize));							//buffer size	
			rm.getDevice().updateDescriptorSets(1, &camWriteDset, 0, nullptr);
		}

	public:
		vk::DeviceSize getBufferSize() const
		{
			return bufferSize;
		}
		vk::Buffer& getUniformBuffer()
		{
			return uniformBuffer;
		}
		vk::DeviceMemory& getUniformBufferMem()
		{
			return uniformBufferMem;
		}
		vk::DescriptorSet& getDescriptorSet() 
		{
			return descriptorSet;
		}
		float getZoom() const
		{
			return zoom;
		}
		void init(uint32_t screenWidth, uint32_t screenHeight, vk::PhysicalDevice gpu, vk::Device device, vk::PhysicalDeviceProperties gpuProperties)
		{
			position = glm::vec3(0.0f, 0.0f, 0.9f);
			lookVector = glm::vec3(0.0f);
			upVector = glm::vec3(0.0f, 1.0f, 0.0f);
			zoom = 1.f;
			attachedMat = nullptr;
			UCBO = {
				glm::ortho(-(float)screenWidth * zoom, (float)screenWidth * zoom, (float)screenHeight * zoom, -(float)screenHeight * zoom, -1.0f, 1.0f),
				glm::mat4(1.0f)//glm::translate(UCBO.camPos, glm::vec3(0.0f, -50.0f, 0.0f))
			}; // proj, camPos
			//UCBO.camPos = glm::scale(UCBO.camPos, glm::vec3(1.f, -1.f, 1.f));
			startingCamPos = UCBO.camPos;
			bufferSize = sizeof(UniformCameraBufferObject);
			isUCBOmapped = false;

			if (bufferSize < gpuProperties.limits.minUniformBufferOffsetAlignment)
				bufferSize = gpuProperties.limits.minUniformBufferOffsetAlignment;
			// camera uniform buffer
			helper.createBuffer(gpu, device, bufferSize,
				vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer,
				vk::MemoryPropertyFlagBits::eHostVisible,
				uniformBuffer, uniformBufferMem);

			// camera uniform buffer mapping
			device.mapMemory(uniformBufferMem, vk::DeviceSize(), bufferSize, vk::MemoryMapFlags(), &data);
			memcpy(data, &UCBO, bufferSize);
			isUCBOmapped = data ? true : false;
		}
		void move(const float x, const float y, const float z = 0.0f)
		{
			UCBO.camPos = glm::translate(UCBO.camPos, glm::vec3(x, y, z));
		}
		void addZoom(const float addZoom, uint32_t screenWidth, uint32_t screenHeight)
		{
			zoom += addZoom;
			if (zoom <= 0)
				zoom = 0.01f;
			UCBO.proj = glm::ortho(-(float)screenWidth * zoom, (float)screenWidth * zoom, (float)screenHeight * zoom, -(float)screenHeight * zoom, -1.0f, 1.0f);
		}
		void update()
		{
			if (!isUCBOmapped)
				return; // do something when not mapped
			float yOffset = 0.0f;
			float xOffset = 0.0f;
			glm::vec4 pos;
			if (attachedMat)
				pos = *attachedMat * glm::vec4(1.0f);
			else
				pos = glm::vec4(position, 1.0f);
			UCBO.camPos = glm::translate(startingCamPos, glm::vec3(-pos.x + xOffset, -pos.y + yOffset, 0.0f));

			memcpy(data, &UCBO, bufferSize);
		}
		void attachTo(glm::mat4 &attachMat) 
		{
			attachedMat = &attachMat;
		}
		void detach()
		{
			attachedMat = nullptr;
			position = glm::vec3(0.0f, 0.0f, 0.9f);
		}
	};
}