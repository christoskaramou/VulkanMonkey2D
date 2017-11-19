#include "ResourceManager.h"
#include "ErrorAndLog.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define MAX_SHAPED_BUFFERS 120
#define DOWNSCALE_CONST 20.0f

namespace vm {
	vk::PhysicalDevice& ResourceManager::getGpu() { return pGpu; }
	vk::Device& ResourceManager::getDevice() { return pDevice; }
	vk::CommandPool& ResourceManager::getCommandPool() { return pCommandPool; }
	vk::Queue & ResourceManager::getGraphicsQueue() { return pGraphicsQueue; }
	vk::PhysicalDeviceProperties& ResourceManager::getGpuProperties() { return pGpuProperties; }
	vk::Extent2D& ResourceManager::getSwapChainExtend2D() { return pSChainExt2D; }

	void ResourceManager::setUpCameraDescriptorSetLayout()
	{
		if (!resourceManagerInitialized) {
			LOG("******Init resource manager first******\n");
			exit(-1);
		}
		auto const camDSLB = vk::DescriptorSetLayoutBinding()
			.setBinding(0) // binding number in shader stages (the set = 1 binding = 0)
			.setDescriptorCount(1) // number of descriptors contained
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex); // which pipeline shader stages can access
		auto const camCreateInfo = vk::DescriptorSetLayoutCreateInfo()
			.setBindingCount(1)
			.setPBindings(&camDSLB);
		errCheck(pDevice.createDescriptorSetLayout(&camCreateInfo, nullptr, &cameraDescriptorSetLayout));
	}

	void ResourceManager::setUpSpriteDescriptorSetLayout()
	{
		if (!resourceManagerInitialized) {
			LOG("******Init resource manager first******\n");
			exit(-1);
		}
		// sprite descriptionSetLayout
		std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBinding{};
		// binding for mvp matrix
		descriptorSetLayoutBinding.push_back(vk::DescriptorSetLayoutBinding()
			.setBinding(0) // binding number in shader stages
			.setDescriptorCount(1) // number of descriptors contained
			.setDescriptorType(vk::DescriptorType::eUniformBufferDynamic)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex)); // which pipeline shader stages can access
															   // binding for texture
		descriptorSetLayoutBinding.push_back(vk::DescriptorSetLayoutBinding()
			.setBinding(1) // binding number in shader stages
			.setDescriptorCount(1) // number of descriptors contained
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setPImmutableSamplers(&spriteSampler)
			.setStageFlags(vk::ShaderStageFlagBits::eFragment)); // which pipeline shader stages can access
		auto const createInfo = vk::DescriptorSetLayoutCreateInfo()
			.setBindingCount((uint32_t)descriptorSetLayoutBinding.size())
			.setPBindings(descriptorSetLayoutBinding.data());
		errCheck(pDevice.createDescriptorSetLayout(&createInfo, nullptr, &spritesDescriptorSetLayout));
	}

	void ResourceManager::setUpPointLightsDescriptorSetLayout()
	{
		if (!resourceManagerInitialized) {
			LOG("******Init resource manager first******\n");
			exit(-1);
		}
		auto const pLightDSLB = vk::DescriptorSetLayoutBinding()
			.setBinding(0) // binding number in shader stages (the set = 2 binding = 0)
			.setDescriptorCount(1) // number of descriptors contained
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setStageFlags(vk::ShaderStageFlagBits::eFragment); // which pipeline shader stages can access
		auto const pLightCreateInfo = vk::DescriptorSetLayoutCreateInfo()
			.setBindingCount(1)
			.setPBindings(&pLightDSLB);
		errCheck(pDevice.createDescriptorSetLayout(&pLightCreateInfo, nullptr, &pointLightsDescriptorSetLayout));
	}

	Texture& ResourceManager::createNewTexture(std::string path)
	{
		if (!resourceManagerInitialized) {
			LOG("******Init resource manager first******\n");
			exit(-1);
		}
		auto check = textures.find(path);
		if (check != textures.end()) {
			return check->second;			// if the same name of the texture is found in resources, associate the texture to this sprite object
		}
		else {
			textures[path] = Texture();
			textures[path].name = path;
		}

		int texWidth, texHeight, texChannels;
		stbi_set_flip_vertically_on_load(true);
		stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		vk::DeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
		}

		vk::Buffer stagingBuffer;
		vk::DeviceMemory stagingBufferMemory;
		helper.createBuffer(pGpu, pDevice, imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(pDevice, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(pDevice, stagingBufferMemory);

		stbi_image_free(pixels);

		helper.createImage(pGpu, pDevice, texWidth, texHeight, vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
			vk::MemoryPropertyFlagBits::eDeviceLocal, textures[path].image, textures[path].imageMem);

		helper.transitionImageLayout(pDevice, pCommandPool, pGraphicsQueue, textures[path].image, vk::ImageLayout::ePreinitialized, vk::ImageLayout::eTransferDstOptimal);
		helper.copyBufferToImage(pDevice, pCommandPool, pGraphicsQueue, stagingBuffer, textures[path].image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		helper.transitionImageLayout(pDevice, pCommandPool, pGraphicsQueue, textures[path].image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

		pDevice.destroyBuffer(stagingBuffer);
		pDevice.freeMemory(stagingBufferMemory);

		// create texture image view ------------------------------------
		helper.createImageView(pDevice, textures[path].image, vk::Format::eR8G8B8A8Unorm, textures[path].imageView);

		return textures[path];
	}

	int ResourceManager::createUserDefinedBuffers(Rect &rect, bool reverseY) {
		if (!resourceManagerInitialized) {
			LOG("******Init resource manager first******\n");
			exit(-1);
		}
		/*if (!rect) {
			LOG("Define a rect for buffer assignment or change the SpriteType\n");
			exit(-1);
		}*/
		float x, y, w, h;
		x = rect.pos.x;
		y = rect.pos.y;
		w = rect.size.x;
		h = rect.size.y;

		for (int i = 0; i < userShapedBuffers.size(); i++) {
			if (!userShapedBuffers[i].occupied) {
				if (reverseY) {
					userShapedBuffers[i].vertices = {
						{ { x-w, y-h, 0.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f } },
						{ { x+w, y-h, 0.0f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 1.0f } },
						{ { x+w, y+h, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } },
						{ { x-w, y+h, 0.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } }
					};
				}
				else
				{
					userShapedBuffers[i].vertices = {
						{ { x-w, y-h, 0.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
						{ { x+w, y-h, 0.0f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 0.0f } },
						{ { x+w, y+h, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 1.0f } },
						{ { x-w, y+h, 0.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } }
					};
				}
				userShapedBuffers[i].indices = { 0, 1, 2, 2, 3, 0 };

				createBuffers(userShapedBuffers[i], &pGpu, &pDevice, &pCommandPool, &pGraphicsQueue);
				userShapedBuffers[i].occupied = true;
				return i;
			}
		}
		return -1;
	}

	void ResourceManager::init(vk::PhysicalDevice& pGpu, vk::Device& pDevice, vk::CommandPool& pCommandPool, vk::Queue& pGraphicsQueue, vk::PhysicalDeviceProperties& pGpuProperties, vk::Extent2D& pSChainExt2D)
	{
		if (resourceManagerInitialized)
			return;
		this->pGpu = pGpu;
		this->pDevice = pDevice;
		this->pCommandPool = pCommandPool;
		this->pGraphicsQueue = pGraphicsQueue;
		this->pGpuProperties = pGpuProperties;
		this->pSChainExt2D = pSChainExt2D;
		resourceManagerInitialized = true;

		createSpriteSampler();

		userShapedBuffers.resize(MAX_SHAPED_BUFFERS);

		createNewTexture("textures/default.jpg");

		// Box2D world and ground creation
		world = new b2World(b2Vec2(0.0f, -5.f));
	}

	void ResourceManager::deInit()
	{
		delete world;
	}

	void ResourceManager::createSpriteSampler()
	{
		if (!resourceManagerInitialized) {
			LOG("******Init resource manager first******\n");
			exit(-1);
		}
		//create sprite image sampler
		auto const createInfo = vk::SamplerCreateInfo()
			.setMagFilter(vk::Filter::eLinear)
			.setMinFilter(vk::Filter::eLinear)
			.setMipmapMode(vk::SamplerMipmapMode::eLinear)
			.setAddressModeU(vk::SamplerAddressMode::eRepeat)
			.setAddressModeV(vk::SamplerAddressMode::eRepeat)
			.setAddressModeW(vk::SamplerAddressMode::eRepeat)
			.setAnisotropyEnable(VK_TRUE)
			.setMaxAnisotropy(16)
			.setCompareEnable(VK_FALSE)
			.setCompareOp(vk::CompareOp::eAlways)
			.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
			.setUnnormalizedCoordinates(VK_FALSE);

		errCheck(pDevice.createSampler(&createInfo, nullptr, &spriteSampler));
	}

	void ResourceManager::createBuffers(ShapedBuffers &sBufffer, vk::PhysicalDevice*gpu, vk::Device *device, vk::CommandPool *commandPool, vk::Queue *graphicsQueue)
	{
		if (!resourceManagerInitialized) {
			LOG("******Init resource manager first******\n");
			exit(-1);
		}
		// ********** vertex buffer **********
		{
			vk::DeviceSize bufferSize = sizeof(sBufffer.vertices[0]) * sBufffer.vertices.size();

			// staging buffer (GPU mem : CPU accessible)
			vk::Buffer stagingBuffer;
			vk::DeviceMemory stagingBufferMemory;
			helper.createBuffer(*gpu, *device, bufferSize,
				vk::BufferUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
				stagingBuffer, stagingBufferMemory);

			// get a pointer to the staging buffer memory and copy the (CPU) data in it
			void* data;
			auto const offset = vk::DeviceSize(); //0
			auto const memFlags = vk::MemoryMapFlags(); //0
			errCheck(device->mapMemory(stagingBufferMemory, offset, bufferSize, memFlags, &data));
			memcpy(data, sBufffer.vertices.data(), (size_t)bufferSize);
			device->unmapMemory(stagingBufferMemory);

			// local device buffer (GPU mem : CPU not accessible)
			helper.createBuffer(*gpu, *device, bufferSize,
				vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
				vk::MemoryPropertyFlagBits::eDeviceLocal,
				sBufffer.vertexBuffer, sBufffer.vertexBufferMem);

			// copy staging buffer to local buffer
			helper.copyBuffer(*device, *commandPool, *graphicsQueue, &stagingBuffer, &sBufffer.vertexBuffer, &bufferSize);
			helper.destroyBuffer(*device, stagingBuffer, stagingBufferMemory);
		}

		// ********** index buffer **********
		{
			vk::DeviceSize bufferSize = sizeof(sBufffer.indices[0]) * sBufffer.indices.size();

			// staging buffer (CPU accessible)
			vk::Buffer stagingBuffer;
			vk::DeviceMemory stagingBufferMem;
			helper.createBuffer(*gpu, *device, bufferSize,
				vk::BufferUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
				stagingBuffer, stagingBufferMem);

			// copy data (CPU mem) to the staging buffer(temporary buffer)
			void* data;
			errCheck(device->mapMemory(stagingBufferMem, vk::DeviceSize(), bufferSize, vk::MemoryMapFlags(), &data));
			memcpy(data, sBufffer.indices.data(), (size_t)bufferSize);
			device->unmapMemory(stagingBufferMem);

			// local device buffer (GPU)
			helper.createBuffer(*gpu, *device, bufferSize,
				vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
				vk::MemoryPropertyFlagBits::eDeviceLocal,
				sBufffer.indexBuffer, sBufffer.indexBufferMem);

			// copy staging buffer to local buffer
			helper.copyBuffer(*device, *commandPool, *graphicsQueue, &stagingBuffer, &sBufffer.indexBuffer, &bufferSize);
			helper.destroyBuffer(*device, stagingBuffer, stagingBufferMem);
		}
	}

	//TODO needs rework!!!
	std::vector<Rect> ResourceManager::getRectsFromImage(std::string location, bool flip_vertically_on_load)
	{
		struct rgb {
			float r;
			float g;
			float b;
			float a;
		};

		std::vector<Rect> rects;
		std::vector<rgb> pixel;

		int width, height, c;
		stbi_set_flip_vertically_on_load(flip_vertically_on_load);
		auto data = stbi_loadf(location.c_str(), &width, &height, &c, 4);
		if (!data) {
			LOG("error loading image\n");
			exit(-1);
		}
		int totalSize = width*height*c;
		int pixels = width * height;
		LOG("\n\nImage Total Size (pixels, width, height, chunksOf) = " << totalSize << " (" << pixels << ", " << width << ", " << height << ", " << c << ")\n");
		pixel.resize(pixels);
		for (auto i = 0; i < pixels; i++)
		{
			pixel[i].r = data[i * 4 + 0];
			pixel[i].g = data[i * 4 + 1];
			pixel[i].b = data[i * 4 + 2];
			pixel[i].a = data[i * 4 + 3];
		}
		int xTemp = 0, yTemp = 0;
		bool passProcess = false;
		for (int i = 0; i < pixels; i++) {
			if (pixel[i].a > 0) {
				for (auto const &r : rects)
				{
					if ((xTemp >= r.pos.x && xTemp <= r.pos.x + r.size.x) && (yTemp >= r.pos.y && yTemp <= r.pos.y + r.size.y)) {
						passProcess = true;
						break;
					}
				}
				if (!passProcess) {
					int a = i;
					do {
						a++;
					} while (pixel[a].a > 0); // TODO: revisit this

					rects.push_back(Rect());
					rects.back().pos.x = (float)xTemp;
					rects.back().pos.y = (float)yTemp;
					rects.back().size.x = (float)a - i;
					a = i;
					do {
						a += width;
					} while (pixel[a].a > 0);

					rects.back().size.y = (float)(a - i) / width;
				}
			}
			if (xTemp < width - 1) {
				xTemp++;
			}
			else {
				xTemp = 0;
				yTemp++;
			}
			passProcess = false;
		}
		for (auto &r : rects) {
			static int i = -1;
			i++;
			LOG("Rect" << i << " (x = " << r.pos.x << ", y = " << r.pos.y << ", width = " << r.size.x << ", height = " << r.size.y << ")\n");
		}
		stbi_image_free(data);
		return rects;
	}
}