#include "Sprite.h"
#include "ErrorAndLog.h"
#include <cstdarg>
#define STB_IMAGE_IMPLEMENTATION
#include <stb-master/stb_image.h>

namespace vm {
	std::vector<Sprite*> Sprite::sprites{};
	Sprite::Sprite(Rect _rect, std::vector<std::string> imagePathNames)
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

		if (!_rect) {
			LOG("define a rect for buffer assignment\n");
			exit(-1);
		}

		float x, y, w, h;
		x = _rect.pos.x;
		y = _rect.pos.y;
		w = _rect.size.x;
		h = _rect.size.y;
		vertices = {
			{ { -w, -h, 0.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f } },
			{ {  w, -h, 0.0f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 1.0f } },
			{ {  w,  h, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } },
			{ { -w,  h, 0.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } }
		};
		indices = { 0, 1, 2, 2, 3, 0 };
		rect = _rect;

		setTextures(imagePathNames);

		descriptorSet = nullptr;

		Sprite::sprites.push_back(this);
	}
	Rect Sprite::getRect() const
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
	void Sprite::setActiveDescriptorSet(unsigned int num)
	{
		if (num < descriptorSets.size())
			descriptorSet = &descriptorSets[num];
	}
	void Sprite::acquireNextImage(uint32_t start, uint32_t end)
	{
		if (descriptorSets.size() <= 1) return;

		static uint32_t num = start;
		if (start < end) {
			if (num + 1 <= end) {
				descriptorSet = &descriptorSets[num + 1];
			}
			else {
				num = start;
				descriptorSet = &descriptorSets[start];
			}
			num++;
		}
		else {
			if (num - 1 >= end) {
				descriptorSet = &descriptorSets[num - 1];
			}
			else {
				num = start;
				descriptorSet = &descriptorSets[start];
			}
			num--;
		}
	}
	void Sprite::setPointerToUniformBufferMem(vk::DeviceMemory& uniformBufferMem)
	{
		ResourceManager &rm = ResourceManager::getInstance();
		// just get the pointer to the spritesUniformBufferMem offset for this sprite
		rm.getDevice().mapMemory(uniformBufferMem, uBuffInfo.offset, uBuffInfo.size, vk::MemoryMapFlags(), &_uniformMemory);
		rm.getDevice().unmapMemory(uniformBufferMem);
	}

	void Sprite::setTextures(const std::vector<std::string>& imagePathNames)
	{
		textures.clear();
		for (auto &path : imagePathNames) {
			if (path == "")
				textures.push_back(createNewTexture("textures/default.jpg"));
			else
				textures.push_back(createNewTexture(path));
		}
	}

	void Sprite::setTextures(const std::vector<Texture>& _textures)
	{
		textures.clear();
		for (auto &t : textures) 
			textures.push_back(t);
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
	void Sprite::createDescriptorSets(const vk::DescriptorPool &descriptorPool)
	{
		// clear the dSets
		descriptorSets.clear();

		for (auto &t : textures) {
			// push a new dSet in vector
			descriptorSets.push_back(vk::DescriptorSet());

			ResourceManager &rm = ResourceManager::getInstance();
			if (!rm.spritesUniformBuffer) exit(-1);

			auto const allocateInfo = vk::DescriptorSetAllocateInfo()
				.setDescriptorPool(descriptorPool)
				.setDescriptorSetCount(1)
				.setPSetLayouts(&rm.spritesDescriptorSetLayout);
			errCheck(rm.getDevice().allocateDescriptorSets(&allocateInfo, &descriptorSets.back()));

			vk::WriteDescriptorSet writeDset[2];
			//----------for mvp-----------
			writeDset[0] = vk::WriteDescriptorSet()
				.setDstSet(descriptorSets.back())										//descriptor set
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
				.setDstSet(descriptorSets.back())										//descriptor set
				.setDstBinding(1)												//binding number in shader
				.setDstArrayElement(0)											//start element in array
				.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)	//descriptor type
				.setDescriptorCount(1)											//descriptor count
				.setPImageInfo(&vk::DescriptorImageInfo()
					.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
					.setImageView(t.imageView)
					.setSampler(rm.spriteSampler));
			// update DescriptorSets
			rm.getDevice().updateDescriptorSets(2, writeDset, 0, nullptr);
		}
		descriptorSet = &descriptorSets.back();
	}

	Texture Sprite::createNewTexture(std::string imagePath)
	{
		ResourceManager &rm = ResourceManager::getInstance();

		// this boosts the loading time and the memory usage if there are multiple same textures
		if (rm.textures.find(imagePath) != rm.textures.end()) {
			return rm.textures[imagePath];
		}
		else rm.textures[imagePath] = Texture();

		Texture &tex = rm.textures[imagePath];
		tex.name = imagePath;

		int texWidth, texHeight, texChannels;
		stbi_set_flip_vertically_on_load(true);
		stbi_uc* pixels = stbi_load(imagePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		vk::DeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
		}

		vk::Buffer stagingBuffer;
		vk::DeviceMemory stagingBufferMemory;
		helper.createBuffer(rm.getGpu(), rm.getDevice(), imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(rm.getDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(rm.getDevice(), stagingBufferMemory);

		stbi_image_free(pixels);

		helper.createImage(rm.getGpu(), rm.getDevice(), texWidth, texHeight, vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
			vk::MemoryPropertyFlagBits::eDeviceLocal, tex.image, tex.imageMem);

		helper.transitionImageLayout(rm.getDevice(), rm.getCommandPool(), rm.getGraphicsQueue(), tex.image, vk::ImageLayout::ePreinitialized, vk::ImageLayout::eTransferDstOptimal);
		helper.copyBufferToImage(rm.getDevice(), rm.getCommandPool(), rm.getGraphicsQueue(), stagingBuffer, tex.image, 0, 0, texWidth, texHeight);
		helper.transitionImageLayout(rm.getDevice(), rm.getCommandPool(), rm.getGraphicsQueue(), tex.image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

		rm.getDevice().destroyBuffer(stagingBuffer);
		rm.getDevice().freeMemory(stagingBufferMemory);

		// create texture image view ------------------------------------
		helper.createImageView(rm.getDevice(), tex.image, vk::Format::eR8G8B8A8Unorm, tex.imageView);

		return tex;
	}
}