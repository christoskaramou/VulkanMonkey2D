#include "Sprite.h"
#include "ErrorAndLog.h"

namespace vm {
	std::vector<Sprite*> Sprite::sprites{};
	Sprite::Sprite(Rect definedRect, std::string textureName)
	{
		ResourceManager &rm = ResourceManager::getInstance();
		static unsigned int spriteNumber = 0;		// every sprite has a unique ID
		spriteID = spriteNumber++;

		vBuffInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer;
		vBuffInfo.size = sizeof(Vertex) * 4;
		vBuffInfo.offset = spriteID * vBuffInfo.size;

		iBuffInfo.usage = vk::BufferUsageFlagBits::eIndexBuffer;
		iBuffInfo.size = sizeof(uint32_t) * 6;
		iBuffInfo.offset = spriteID * iBuffInfo.size;

		uBuffInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer;
		uBuffInfo.size = sizeof(UniformBufferObject);
		if (uBuffInfo.size < rm.getGpuProperties().limits.minUniformBufferOffsetAlignment)
			uBuffInfo.size = rm.getGpuProperties().limits.minUniformBufferOffsetAlignment;
		uBuffInfo.offset = spriteID * uBuffInfo.size;

		isSpriteMapped = false;
		needsUpdate = false;
		ubo.model = glm::mat4(1.0f);

		type = SpriteType::userDefinedRect;

		if (!definedRect) {
			LOG("define a rect for buffer assignment\n");
			exit(-1);
		}

		float x, y, w, h;
		x = definedRect.pos.x;
		y = definedRect.pos.y;
		w = definedRect.size.x;
		h = definedRect.size.y;
		vertices = {
			{ { -w, -h, 0.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f } },
			{ {  w, -h, 0.0f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 1.0f } },
			{ {  w,  h, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } },
			{ { -w,  h, 0.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } }
		};
		indices = { 0, 1, 2, 2, 3, 0 };
		rect = definedRect;
		if (textureName == "")
			setTexture("textures/default.jpg"); // this also checks if the Texture exists
		else
			setTexture(textureName);

		Sprite::sprites.push_back(this);
	}
	Rect Sprite::getRect()
	{
		return rect;
	}
	bool Sprite::isMapped() const 
	{ 
		return isSpriteMapped;
	}
	SpriteType Sprite::getSpriteType() const
	{ 
		return type;
	}
	unsigned int Sprite::getSpriteID() const 
	{ 
		return spriteID;
	}
	void Sprite::setPointerToUniformBufferMem(vk::DeviceMemory& uniformBufferMem)
	{
		ResourceManager &rm = ResourceManager::getInstance();
		// just get the pointer to the spritesUniformBufferMem offset for this sprite
		rm.getDevice().mapMemory(uniformBufferMem, uBuffInfo.offset, uBuffInfo.size, vk::MemoryMapFlags(), &_uniformMemory);
		rm.getDevice().unmapMemory(uniformBufferMem);
	}

	void Sprite::setTexture(std::string path) // it will only work before descriptorSet is defined!!
	{
		ResourceManager &rm = ResourceManager::getInstance();
		texture = rm.createNewTexture(path);
	}

	void Sprite::setModelPos(glm::mat4 modelPos)
	{
		if (ubo.model == modelPos) 
			return;
		ubo.model = modelPos;
		needsUpdate = true;
	}

	void Sprite::update()
	{
		if (!needsUpdate)
			return;
		// DYNAMIC
		if (type == SpriteType::userDefinedRect)
		{
			ResourceManager &rm = ResourceManager::getInstance();
			if (!_uniformMemory) {
				LOG("Uniform Buffer Memory of Sprite: " << spriteID << " is not mapped\n");
				return;
			}
			memcpy(_uniformMemory, &ubo, uBuffInfo.size);
		}
		// STATIC   // TODO needs rework
		else if (type == SpriteType::userDefinedRectStatic) {
			ResourceManager &rm = ResourceManager::getInstance();
			vk::Buffer stagingBuffer;
			vk::DeviceMemory stagingBufferMemory;
			helper.createBuffer(rm.getGpu(), rm.getDevice(), uBuffInfo.size,
				vk::BufferUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, // eHostCoherent means no cache and there is no need flushing the memory
				stagingBuffer, stagingBufferMemory);

			rm.getDevice().mapMemory(stagingBufferMemory, vk::DeviceSize(), uBuffInfo.size, vk::MemoryMapFlags(), &_uniformMemory);
			memcpy(_uniformMemory, &ubo, uBuffInfo.size);
			rm.getDevice().unmapMemory(stagingBufferMemory);

			helper.copyBuffer(rm.getDevice(), rm.getCommandPool(), rm.getGraphicsQueue(), &stagingBuffer, &rm.spritesUniformBuffer, &uBuffInfo.size);
			isSpriteMapped = false;

			helper.destroyBuffer(rm.getDevice(), stagingBuffer, stagingBufferMemory);
			LOG("Updating static object has a critical drop on frames/sec. Not wise...\n");
		}
		else {
			LOG("Unsupported sprite type\n");
			exit(-1);
		}
		needsUpdate = false;
	}
	void Sprite::createDescriptorSet(const vk::DescriptorPool &descriptorPool, const Texture &texture)
	{
		ResourceManager &rm = ResourceManager::getInstance();
		if (!rm.spritesUniformBuffer) exit(-1);

		auto const allocateInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(descriptorPool)
			.setDescriptorSetCount(1)
			.setPSetLayouts(&rm.spritesDescriptorSetLayout);
		errCheck(rm.getDevice().allocateDescriptorSets(&allocateInfo, &descriptorSet));

		vk::WriteDescriptorSet writeDset[2];
		//----------for mvp-----------
		writeDset[0] = vk::WriteDescriptorSet()
			.setDstSet(descriptorSet)										//descriptor set
			.setDstBinding(0)												//binding number in shader
			.setDstArrayElement(0)											//start element in array
			.setDescriptorType(vk::DescriptorType::eUniformBufferDynamic)	//descriptor type
			.setDescriptorCount(1)											//descriptor count
			.setPBufferInfo(&vk::DescriptorBufferInfo()
				.setBuffer(rm.spritesUniformBuffer)								//buffer
				.setOffset(0)													//buffer offset
				.setRange(uBuffInfo.size));										//buffer size	
		//----------for textures-------
		writeDset[1] = vk::WriteDescriptorSet()
			.setDstSet(descriptorSet)										//descriptor set
			.setDstBinding(1)												//binding number in shader
			.setDstArrayElement(0)											//start element in array
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)	//descriptor type
			.setDescriptorCount(1)											//descriptor count
			.setPImageInfo(&vk::DescriptorImageInfo()
				.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
				.setImageView(texture.imageView)
				.setSampler(rm.spriteSampler));
		// update DescriptorSets
		rm.getDevice().updateDescriptorSets(2, writeDset, 0, nullptr);
	}
}