#include "Renderer.h"
#include "ErrorAndLog.h"
#include <chrono>
#include <random>
#include <string>

#define APP_SHORT_NAME "VulkanMonkey"

namespace vm {

	Renderer::Renderer(GLFWwindow * _window) : window(_window)
	{
		allocateConsole();

		enableLayersAndExtensions();

		createInstance();
		createSurface();
		selectPhysicalDevice();
		createLogicalDevice();

		createSwapchain();
		createImageViews();

		createCommandPool();

		createRenderPass();
		createDepthResources();
		createFrameBuffers();

		createCommandBuffers();

		//init ResourceManager
		ResourceManager::getInstance().init(gpu, device, commandPool, graphicsQueue, gpuProperties, swapchainExtent);

		createDescriptorSetLayout();
		createGraphicsPipeline();

		
	}
	Renderer::~Renderer()
	{
		device.waitIdle();
		Sprite::sprites.clear();
		destroySemaphores();

		destroyDescriptorPool(); // Descriptor sets are destroyed when destroying the descriptor pool
		destroyTextures();
		destroyUniformBuffers();

		destroyIndexBuffers();
		destroyVertexBuffers();

		destroyGraphicsPipeline();
		destroyDescriptorSetLayout();

		destroyFrameBuffers();
		destroyDepthResources();
		destroyRenderPass();

		destroyCommandPool(); // cmd buffers are auto destroyed with cmd pool
		destroyImageViews();
		destroySwapchain();

		destroyLogicalDevice();
		destroySurface();
		destroyInstance();

		freeConsole();
	}
	void Renderer::enableLayersAndExtensions()
	{
		instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
		instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
		deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

#ifdef _DEBUG
		instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif
	}
	Camera* Renderer::getMainCamera()
	{
		return &mainCamera;
	}
	void Renderer::createInstance()
	{
#ifdef _DEBUG
		reportInfo = vk::DebugReportCallbackCreateInfoEXT()
			.setFlags(
				vk::DebugReportFlagBitsEXT::eInformation |
				vk::DebugReportFlagBitsEXT::eWarning |
				vk::DebugReportFlagBitsEXT::ePerformanceWarning |
				vk::DebugReportFlagBitsEXT::eError |
				vk::DebugReportFlagBitsEXT::eDebug)
			.setPfnCallback(VulkanDebugCallBack);
#endif
		auto const appInfo = vk::ApplicationInfo()
			.setPApplicationName(APP_SHORT_NAME)
			.setApplicationVersion(0)
			.setPEngineName(APP_SHORT_NAME)
			.setEngineVersion(0)
			.setApiVersion(VK_API_VERSION_1_0);
		auto const instInfo = vk::InstanceCreateInfo()
			.setPApplicationInfo(&appInfo)
			.setEnabledLayerCount((uint32_t)(instanceLayers.size()))
			.setPpEnabledLayerNames(instanceLayers.data())
			.setEnabledExtensionCount((uint32_t)(instanceExtensions.size()))
			.setPpEnabledExtensionNames(instanceExtensions.data())
#ifdef _DEBUG
			.setPNext(&reportInfo);
#else
			.setPNext(nullptr);
#endif

		errCheck(vk::createInstance(&instInfo, nullptr, &instance));
		setUpDebugCallback(); //autoruns only in debug mode
	}
	void Renderer::destroyInstance()
	{
		destroyDebugCallback(); // autoruns only in debug mode
		instance.destroy();
	}
	void Renderer::selectPhysicalDevice()
	{
		uint32_t gpuCount = 0;
		errCheck(instance.enumeratePhysicalDevices(&gpuCount, nullptr));
		std::vector<vk::PhysicalDevice> gpuList(gpuCount);
		errCheck(instance.enumeratePhysicalDevices(&gpuCount, gpuList.data()));

		for (const auto& device : gpuList) {
			if (isDeviceSuitable(device, surface)) {
				gpu = device;
				break;
			}
		}
		if (!gpu) {
			LOG("No GPU wants you here!\n");
			exit(-1);
		}
		gpu.getProperties(&gpuProperties);
		gpu.getFeatures(&gpuFeatures);
	}
	bool Renderer::isDeviceSuitable(vk::PhysicalDevice gpu, vk::SurfaceKHR surface)
	{
		VulkanQueueFamily queueFamily;
		queueFamily.findQueueFamilies(gpu, surface);
		return queueFamily.isComplete();
	}
	void VulkanQueueFamily::findQueueFamilies(vk::PhysicalDevice gpu, vk::SurfaceKHR surface)
	{
		uint32_t familyCount = 0;
		gpu.getQueueFamilyProperties(&familyCount, nullptr);
		std::vector<vk::QueueFamilyProperties> properties(familyCount);
		gpu.getQueueFamilyProperties(&familyCount, properties.data());

		for (uint32_t i = 0; i < familyCount; ++i) {
			//find graphics queue family index
			if (properties[i].queueFlags & vk::QueueFlagBits::eGraphics) {
				graphicsFamilyId = i;
			}
			// find present queue family index
			vk::Bool32 presentSupport = false;
			errCheck(gpu.getSurfaceSupportKHR(i, surface, &presentSupport));
			if (properties[i].queueCount > 0 && presentSupport) {
				presentFamilyId = i;
			}

			if (isComplete()) {
				break;
			}
		}
	}
	void Renderer::createLogicalDevice()
	{
		VulkanQueueFamily queueFamily;
		queueFamily.findQueueFamilies(gpu, surface);
		float priorities[]{ 1.0f }; // range : [0.0, 1.0]
		auto const queueCreateInfo = vk::DeviceQueueCreateInfo()
			.setQueueFamilyIndex(queueFamily.graphicsFamilyId)
			.setQueueCount(1)
			.setPQueuePriorities(priorities);

		auto const deviceCreateInfo = vk::DeviceCreateInfo()
			.setQueueCreateInfoCount(1)
			.setPQueueCreateInfos(&queueCreateInfo)
			.setEnabledLayerCount((uint32_t)deviceLayers.size())
			.setPpEnabledLayerNames(deviceLayers.data())
			.setEnabledExtensionCount((uint32_t)deviceExtensions.size())
			.setPpEnabledExtensionNames(deviceExtensions.data())
			.setPEnabledFeatures(&gpuFeatures);

		errCheck(gpu.createDevice(&deviceCreateInfo, nullptr, &device));

		//get the graphics queue handler
		graphicsQueue = device.getQueue(queueFamily.graphicsFamilyId, 0);
		presentQueue = device.getQueue(queueFamily.presentFamilyId, 0);

		uint32_t count = 0;
		errCheck(gpu.enumerateDeviceExtensionProperties(nullptr, &count, nullptr));
		std::vector<vk::ExtensionProperties> ep(count);
		errCheck(gpu.enumerateDeviceExtensionProperties(nullptr, &count, ep.data()));
	}
	void Renderer::destroyLogicalDevice()
	{
		device.destroy();
	}
	void Renderer::createSurface()
	{
		VkSurfaceKHR surf;
		glfwCreateWindowSurface(instance, window, nullptr, &surf);
		surface = surf;
	}
	void Renderer::destroySurface()
	{
		instance.destroySurfaceKHR(surface);
	}
	void Renderer::createSwapchain()
	{
		SwapchainInfo swapchainInfo;
		swapchainInfo.querySwapChainSupport(gpu, surface);

		surfaceCapabilitiesKHR = swapchainInfo.capabilities;
		surfaceFormatKHR = swapchainInfo.chooseSwapchainFormat();
		presentModeKHR = swapchainInfo.chooseSwapchainPresentMode();
		swapchainExtent = swapchainInfo.chooseSwapchainExtent();

		swapchainImageCount = surfaceCapabilitiesKHR.minImageCount + 1;
		if (surfaceCapabilitiesKHR.maxImageCount > 0 &&
			swapchainImageCount > surfaceCapabilitiesKHR.maxImageCount) {
			swapchainImageCount = surfaceCapabilitiesKHR.maxImageCount;
		}

		VulkanQueueFamily queueFamily;
		queueFamily.findQueueFamilies(gpu, surface);
		uint32_t queueFamilyIndicies[] = { (uint32_t)queueFamily.graphicsFamilyId, (uint32_t)queueFamily.presentFamilyId };

		vk::SwapchainKHR oldSwapchain = swapchain;//changes
		auto const swapchainCreateInfo = vk::SwapchainCreateInfoKHR()
			.setSurface(surface)
			.setMinImageCount(swapchainImageCount)
			.setImageFormat(surfaceFormatKHR.format)
			.setImageColorSpace(surfaceFormatKHR.colorSpace)
			.setImageExtent(swapchainExtent)
			.setImageArrayLayers(1)
			.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
			.setPreTransform(surfaceCapabilitiesKHR.currentTransform)
			.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
			.setPresentMode(presentModeKHR)
			.setClipped(VK_TRUE)
			.setOldSwapchain(oldSwapchain);//changes
		vk::SwapchainKHR newSwapchain;//changes
		errCheck(device.createSwapchainKHR(&swapchainCreateInfo, nullptr, &newSwapchain));//changes

		if (swapchain)//changes
			device.destroySwapchainKHR(swapchain);//changes
		swapchain = newSwapchain;//changes

								 // get the handlers
		errCheck(device.getSwapchainImagesKHR(swapchain, &swapchainImageCount, nullptr));
		swapchainImages.resize(swapchainImageCount);
		errCheck(device.getSwapchainImagesKHR(swapchain, &swapchainImageCount, swapchainImages.data()));
	}
	void Renderer::destroySwapchain()
	{
		device.destroySwapchainKHR(swapchain, nullptr);
	}
	void Renderer::createImageViews()
	{
		swapchainImageViews.resize(swapchainImageCount);
		// create image views for each swapchain image
		for (uint32_t i = 0; i < swapchainImageCount; ++i) {
			auto const imageViewCreateInfo = vk::ImageViewCreateInfo()
				.setImage(swapchainImages[i])
				.setViewType(vk::ImageViewType::e2D)
				.setFormat(surfaceFormatKHR.format)
				.setComponents(vk::ComponentMapping()
					.setR(vk::ComponentSwizzle::eIdentity)
					.setG(vk::ComponentSwizzle::eIdentity)
					.setB(vk::ComponentSwizzle::eIdentity)
					.setA(vk::ComponentSwizzle::eIdentity))
				.setSubresourceRange(vk::ImageSubresourceRange()
					.setAspectMask(vk::ImageAspectFlagBits::eColor)
					.setBaseMipLevel(0)
					.setLevelCount(1)
					.setBaseArrayLayer(0)
					.setLayerCount(1));
			errCheck(device.createImageView(&imageViewCreateInfo, nullptr, &swapchainImageViews[i]));
		}
	}
	void Renderer::destroyImageViews()
	{
		for (auto iView : swapchainImageViews)
			device.destroyImageView(iView);
	}
	void Renderer::createTextures(std::vector<std::string> &pFile)
	{
	}
	void Renderer::destroyTextures()
	{
		ResourceManager &rm = ResourceManager::getInstance();
		for (auto &x : rm.textures) {
			helper.destroyImage(device, x.second.image, x.second.imageMem);
			device.destroyImageView(x.second.imageView);
		}
		device.destroySampler(rm.spriteSampler);
	}
	void Renderer::createTextureImageViews()
	{
	}
	void Renderer::destroyTextureImageViews()
	{
	}
	void Renderer::createTextureSamplers()
	{
	}
	void Renderer::destroyTextureSamplers()
	{
	}
	void Renderer::reInitSwapchain()
	{
		if (device)
			device.waitIdle();
		if (!swapchain)
			return;

		destroyDepthResources();
		destroyFrameBuffers();
		device.freeCommandBuffers(commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
		destroyGraphicsPipeline();
		destroyRenderPass();
		destroyImageViews();

		createSwapchain();
		createImageViews();			// swapchain image update
		createRenderPass();			// swapchain image format update
		createGraphicsPipeline();	// viewport scissor update
		createDepthResources();		// match the new color attachment resolution
		createFrameBuffers();		// swapchain image update
		createCommandBuffers();		// swapchain image update
		recordCommandBuffers();
	}
	std::string Renderer::getGpuName()
	{
		return gpuProperties.deviceName;
	}
	void Renderer::createRenderPass()
	{
		auto const cad = vk::AttachmentDescription() // color attachment disc
			.setFormat(surfaceFormatKHR.format)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

		auto const car = vk::AttachmentReference() // color attachment ref
			.setAttachment(0)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

		auto const depthAttachment = vk::AttachmentDescription()
			.setFormat(helper.findDepthFormat(gpu))
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

		auto const depthAttachmentRef = vk::AttachmentReference()
			.setAttachment(1)
			.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

		auto const sd = vk::SubpassDescription() // subpass desc (there can be multiple subpasses)
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachmentCount(1)
			.setPColorAttachments(&car)
			.setPDepthStencilAttachment(&depthAttachmentRef);

		auto const dependency = vk::SubpassDependency()
			.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setDstSubpass(0)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setSrcAccessMask(vk::AccessFlags())
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

		std::array<vk::AttachmentDescription, 2> attachments = { cad, depthAttachment };

		auto const rpci = vk::RenderPassCreateInfo()
			.setAttachmentCount(static_cast<uint32_t>(attachments.size()))
			.setPAttachments(attachments.data())
			.setSubpassCount(1)
			.setPSubpasses(&sd)
			.setDependencyCount(1)
			.setPDependencies(&dependency);

		errCheck(device.createRenderPass(&rpci, nullptr, &renderPass));
	}
	void Renderer::destroyRenderPass()
	{
		device.destroyRenderPass(renderPass);
	}
	void Renderer::createFrameBuffers()
	{
		swapchainFrameBuffers.resize(swapchainImageViews.size());

		for (size_t i = 0; i < swapchainImageViews.size(); ++i) {
			std::array<vk::ImageView, 2> attachments = { swapchainImageViews[i], depthImageView };

			auto const fbci = vk::FramebufferCreateInfo()
				.setRenderPass(renderPass)
				.setAttachmentCount(static_cast<uint32_t>(attachments.size()))
				.setPAttachments(attachments.data())
				.setWidth(swapchainExtent.width)
				.setHeight(swapchainExtent.height)
				.setLayers(1);
			errCheck(device.createFramebuffer(&fbci, nullptr, &swapchainFrameBuffers[i]));
		}
	}
	void Renderer::destroyFrameBuffers()
	{
		for (auto scb : swapchainFrameBuffers)
			device.destroyFramebuffer(scb);
	}
	void Renderer::createVertexBuffers()
	{
		ResourceManager &rm = ResourceManager::getInstance();

		// SPRITES VERTEX BUFFER
		// local device buffer (GPU mem : CPU not accessible)
		vk::DeviceSize vBufSize = Sprite::sprites.size() * sizeof(Vertex) * 4;
		helper.createBuffer(gpu, device, vBufSize,
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal,
			rm.spritesVertexBuffer, rm.spritesVertexBufferMem);
		
		std::vector<Vertex> verts{};
		for (auto &s : Sprite::sprites) {
			verts.push_back(s->vertices[0]);
			verts.push_back(s->vertices[1]);
			verts.push_back(s->vertices[2]);
			verts.push_back(s->vertices[3]);
		}
		// staging buffer (GPU mem : CPU accessible)
		vk::Buffer stagingBuffer;
		vk::DeviceMemory stagingBufferMemory;
		helper.createBuffer(gpu, device, vBufSize,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
			stagingBuffer, stagingBufferMemory);

		// get a pointer to the staging buffer memory and copy the (CPU) data in it
		void * data;
		auto const offset = vk::DeviceSize();
		auto const memFlags = vk::MemoryMapFlags(); //0
		device.mapMemory(stagingBufferMemory, offset, vBufSize, memFlags, &data);
		memcpy(data, verts.data(), (size_t)vBufSize);
		device.unmapMemory(stagingBufferMemory);

		// copy staging buffer to local buffer
		helper.copyBuffer(device, commandPool, graphicsQueue, &stagingBuffer, &rm.spritesVertexBuffer, &vBufSize);
		helper.destroyBuffer(device, stagingBuffer, stagingBufferMemory);
	}
	void Renderer::destroyVertexBuffers()
	{
		ResourceManager &rm = ResourceManager::getInstance();
		for (auto &x : rm.userShapedBuffers)
			helper.destroyBuffer(device, x.vertexBuffer, x.vertexBufferMem);
		helper.destroyBuffer(device, rm.spritesVertexBuffer, rm.spritesVertexBufferMem);
	}
	void Renderer::createIndexBuffers()
	{
		ResourceManager &rm = ResourceManager::getInstance();

		// SPRITES INDEX BUFFER
		// local device buffer (GPU mem : CPU not accessible)
		vk::DeviceSize iBufSize = sizeof(uint32_t) * 6;
		helper.createBuffer(gpu, device, iBufSize,
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal,
			rm.spritesIndexBuffer, rm.spritesIndexBufferMem);

		// staging buffer (GPU mem : CPU accessible)
		vk::Buffer stagingBuffer;
		vk::DeviceMemory stagingBufferMemory;
		helper.createBuffer(gpu, device, iBufSize,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
			stagingBuffer, stagingBufferMemory);

		// get a pointer to the staging buffer memory and copy the (CPU) data in it
		void * data;
		auto const offset = vk::DeviceSize();
		auto const memFlags = vk::MemoryMapFlags(); //0
		device.mapMemory(stagingBufferMemory, offset, iBufSize, memFlags, &data);
		memcpy(data, Sprite::sprites[0]->indices.data(), (size_t)iBufSize);
		device.unmapMemory(stagingBufferMemory);

		// copy staging buffer to local buffer
		helper.copyBuffer(device, commandPool, graphicsQueue, &stagingBuffer, &rm.spritesIndexBuffer, &iBufSize);
		helper.destroyBuffer(device, stagingBuffer, stagingBufferMemory);
	}
	void Renderer::destroyIndexBuffers()
	{
		ResourceManager &rm = ResourceManager::getInstance();
		for (auto &x : rm.userShapedBuffers)
			helper.destroyBuffer(device, x.indexBuffer, x.indexBufferMem);
		helper.destroyBuffer(device, rm.spritesIndexBuffer, rm.spritesIndexBufferMem);
	}
	void Renderer::createUniformBuffers()
	{
		//SPRITES UNIFORM BUFFER
		ResourceManager &rm = ResourceManager::getInstance();
		std::vector<UniformBufferObject> ubos;
		for (auto &s : Sprite::sprites) {
			ubos.push_back(s->ubo);
		}
		vk::DeviceSize uniSize = Sprite::sprites.size() * Sprite::sprites[0]->uBuffInfo.size;
		helper.createBuffer(gpu, device, uniSize,
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible, //vk::MemoryPropertyFlagBits::eDeviceLocal
			rm.spritesUniformBuffer, rm.spritesUniformBufferMem);

		// get the pointers to the later one big uniform mapped memory to avoid mapping-unmpapping each sprite,
		// persistent mapping gives a good boost if there are multiple sprites
		for (auto &s : Sprite::sprites)
			s->setPointerToUniformBufferMem(rm.spritesUniformBufferMem);

		// persistent mapping
		errCheck(device.mapMemory(rm.spritesUniformBufferMem, vk::DeviceSize(), uniSize, vk::MemoryMapFlags(), &rm.spritesUniformData));
		memcpy(rm.spritesUniformData, ubos.data(), uniSize);
		//device.unmapMemory(rm.spritesUniformBufferMem);
	}
	void Renderer::destroyUniformBuffers()
	{
		helper.destroyBuffer(device, ResourceManager::getInstance().spritesUniformBuffer, ResourceManager::getInstance().spritesUniformBufferMem);

		device.unmapMemory(mainCamera.getUniformBufferMem());
		helper.destroyBuffer(device, mainCamera.getUniformBuffer(), mainCamera.getUniformBufferMem());
	}
	void Renderer::createCommandPool()
	{
		VulkanQueueFamily qi;
		qi.findQueueFamilies(gpu, surface);

		auto const cpci = vk::CommandPoolCreateInfo()
			.setQueueFamilyIndex(qi.graphicsFamilyId)
			.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
		errCheck(device.createCommandPool(&cpci, nullptr, &commandPool));
	}
	void Renderer::createCommandBuffers()
	{
		commandBuffers.resize(swapchainFrameBuffers.size());

		auto const cbai = vk::CommandBufferAllocateInfo()
			.setCommandPool(commandPool)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount((uint32_t)(commandBuffers.size()));
		errCheck(device.allocateCommandBuffers(&cbai, commandBuffers.data()));
	}
	void Renderer::destroyCommandPool()
	{
		device.destroyCommandPool(commandPool);
	}
	void Renderer::createStagingUniformBuffers()
	{
	}
	void Renderer::destroyStagingUniformBuffers()
	{
	}
	void Renderer::initUniformBuffers()
	{
	}
	void Renderer::updateUniformBuffer(int obj)
	{
	}
	void Renderer::createDepthResources()
	{
		vk::Format depthFormat = helper.findDepthFormat(gpu);
		helper.createImage(gpu, device, swapchainExtent.width, swapchainExtent.height, depthFormat,
			vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment,
			vk::MemoryPropertyFlagBits::eDeviceLocal, depthImage, depthImageMemory);
		helper.createImageView(device, depthImage, depthFormat, depthImageView, vk::ImageAspectFlagBits::eDepth);
		helper.transitionImageLayout(device, commandPool, graphicsQueue, depthImage,
			vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal, depthFormat);

	}
	void Renderer::destroyDepthResources()
	{
		device.destroyImageView(depthImageView);
		device.destroyImage(depthImage);
		device.freeMemory(depthImageMemory);
	}
	void Renderer::createDescriptorPool()
	{
		std::vector<vk::DescriptorPoolSize> descriptorPoolSizes = {};

		// for mvp uniform
		descriptorPoolSizes.push_back(vk::DescriptorPoolSize()
			.setType(vk::DescriptorType::eUniformBufferDynamic)	//descriptor type
			.setDescriptorCount((uint32_t)Sprite::sprites.size() + 2));		//descriptor count

		// for texture
		descriptorPoolSizes.push_back(vk::DescriptorPoolSize()
			.setType(vk::DescriptorType::eCombinedImageSampler)	//descriptor type
			.setDescriptorCount((uint32_t)Sprite::sprites.size() + 2));	//descriptor count

		// for camera
		descriptorPoolSizes.push_back(vk::DescriptorPoolSize()
			.setType(vk::DescriptorType::eUniformBuffer)	//descriptor type
			.setDescriptorCount(1));						//descriptor count

		auto const createInfo = vk::DescriptorPoolCreateInfo()
			.setPoolSizeCount((uint32_t)descriptorPoolSizes.size())
			.setPPoolSizes(descriptorPoolSizes.data())
			.setMaxSets((uint32_t)Sprite::sprites.size() + 5);

		errCheck(device.createDescriptorPool(&createInfo, nullptr, &descriptorPool));
	}
	void Renderer::destroyDescriptorPool()
	{
		device.destroyDescriptorPool(descriptorPool);
	}
	void Renderer::createDescriptorSet()
	{
		ResourceManager &rm = ResourceManager::getInstance();

		//standard uniform buffer size
		vk::DeviceSize uBufferSize = sizeof(UniformBufferObject);
		if (uBufferSize < rm.getGpuProperties().limits.minUniformBufferOffsetAlignment)
			uBufferSize = rm.getGpuProperties().limits.minUniformBufferOffsetAlignment;

		for (auto &s : Sprite::sprites) {
			s->createDescriptorSet(descriptorPool, s->texture);
		}

		mainCamera.createDescriptorSet(descriptorPool);

	}
	void Renderer::createDescriptorSetLayout()
	{
		// camera descriptionSetLayout
		ResourceManager::getInstance().setUpCameraDescriptorSetLayout();

		// sprite descriptionSetLayout
		ResourceManager::getInstance().setUpSpriteDescriptorSetLayout();
	}
	void Renderer::destroyDescriptorSetLayout()
	{
		for (auto &ds : descriptorSetLayouts)
			device.destroyDescriptorSetLayout(ds);
		device.destroyDescriptorSetLayout(ResourceManager::getInstance().spritesDescriptorSetLayout);
		device.destroyDescriptorSetLayout(ResourceManager::getInstance().cameraDescriptorSetLayout);
	}
	void Renderer::createGraphicsPipeline()
	{
		vk::ShaderModule vShaderMod; //dont forget to destroy this
		vk::ShaderModule fShaderMod; //dont forget to destroy this

		{
			auto vertShaderCode = readFile("shaders/shader.vert.spv");
			auto fragShaderCode = readFile("shaders/shader.frag.spv");
			//std::cout << "\t\tShaders loaded: Size {" << vertShaderCode.size() << ", " << fragShaderCode.size() << "}\n";

			createShaderModule(vertShaderCode, vShaderMod);
			createShaderModule(fragShaderCode, fShaderMod);
		}
		auto vssi = vk::PipelineShaderStageCreateInfo() //vertex shader stage create info
			.setStage(vk::ShaderStageFlagBits::eVertex)
			.setModule(vShaderMod)
			.setPName("main")
			.setPSpecializationInfo(nullptr); //values for shader constants
		auto fssi = vk::PipelineShaderStageCreateInfo() //frag shader stage create info
			.setStage(vk::ShaderStageFlagBits::eFragment)
			.setModule(fShaderMod)
			.setPName("main")
			.setPSpecializationInfo(nullptr); //values for shader constants
		vk::PipelineShaderStageCreateInfo shaderStages[] = { vssi, fssi }; //shader stages holder

																		   //1. Vertex input stage [Fixed]
		auto bindingDiscription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescription();
		auto visci = vk::PipelineVertexInputStateCreateInfo()
			.setVertexBindingDescriptionCount(1)
			.setPVertexBindingDescriptions(&bindingDiscription)
			.setVertexAttributeDescriptionCount((uint32_t)attributeDescriptions.size())
			.setPVertexAttributeDescriptions(attributeDescriptions.data());

		//2. Input assembly stage [Fixed]
		auto iasci = vk::PipelineInputAssemblyStateCreateInfo()
			.setTopology(vk::PrimitiveTopology::eTriangleList)
			.setPrimitiveRestartEnable(VK_FALSE);

		//3. Vertex shader stage [Programmable]

		//4. Tessellation stage [Programmable] : optional

		//5. Geometry shader stage [Programmable] : optional

		//6-1. Viewports and scissors [Fixed]
		auto viewport = vk::Viewport() //region of the frambuffer
			.setX(0.0f)
			.setY(0.0f)
			.setWidth((float)swapchainExtent.width)
			.setHeight((float)swapchainExtent.height)
			.setMinDepth(-1.0f)
			.setMaxDepth(1.0f);
		auto scissor = vk::Rect2D()
			.setOffset({ 0, 0 }) //vk::Offset2D() constructor also auto sets to {0, 0}
			.setExtent(swapchainExtent);
		auto viewportState = vk::PipelineViewportStateCreateInfo()
			.setViewportCount(1)
			.setPViewports(&viewport)
			.setScissorCount(1)
			.setPScissors(&scissor);

		//6-2. Rasterizer stage [Fixed]
		auto rasterizer = vk::PipelineRasterizationStateCreateInfo()
			.setDepthClampEnable(VK_FALSE)
			.setRasterizerDiscardEnable(VK_FALSE)
			.setPolygonMode(vk::PolygonMode::eFill)
			.setLineWidth(1.0f)
			.setCullMode(vk::CullModeFlagBits::eBack)
			//.setFrontFace(vk::FrontFace::eClockwise)
			.setFrontFace(vk::FrontFace::eCounterClockwise)
			.setDepthBiasEnable(VK_FALSE)
			.setDepthBiasConstantFactor(0.0f) //optional
			.setDepthBiasClamp(0.0f) //optional
			.setDepthBiasSlopeFactor(0.0f); //optional

											//6-3. Multisampling stage [Fixed]
		auto multisampling = vk::PipelineMultisampleStateCreateInfo()
			.setSampleShadingEnable(VK_FALSE)
			.setRasterizationSamples(vk::SampleCountFlagBits::e1)
			.setMinSampleShading(1.0f) //optional
			.setPSampleMask(nullptr) //optional
			.setAlphaToCoverageEnable(VK_FALSE) //optional
			.setAlphaToOneEnable(VK_FALSE); //optional

		//6-4. Depth and stencil testing [Fixed]
		auto depthStencil = vk::PipelineDepthStencilStateCreateInfo()
			.setDepthTestEnable(VK_TRUE)
			.setDepthWriteEnable(VK_TRUE)
			.setDepthCompareOp(vk::CompareOp::eLess)
			.setDepthBoundsTestEnable(VK_FALSE) // for the optional depth bound test
			.setMinDepthBounds(0.0f) // for the optional depth bound test
			.setMaxDepthBounds(1.0f) // for the optional depth bound test
			.setStencilTestEnable(VK_FALSE) // for the optional stencil test
			.setFront(vk::StencilOpState()) // for the optional stencil test
			.setBack(vk::StencilOpState()); // for the optional stencil test

		//7. Fragment shader stage [Programmable]

		//8. Color blending [Fixed]
		auto colorAttachment = vk::PipelineColorBlendAttachmentState()
			.setColorWriteMask(
				vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB |
				vk::ColorComponentFlagBits::eA)
			.setBlendEnable(VK_TRUE)
			.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
			.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
			.setColorBlendOp(vk::BlendOp::eAdd)
			.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
			.setDstAlphaBlendFactor(vk::BlendFactor::eZero)
			.setAlphaBlendOp(vk::BlendOp::eAdd);
		auto colorBlending = vk::PipelineColorBlendStateCreateInfo()
			.setLogicOpEnable(VK_TRUE)
			.setLogicOp(vk::LogicOp::eCopy)
			.setAttachmentCount(1)
			.setPAttachments(&colorAttachment)
			.setBlendConstants({ 1.0f, 0.0f, 0.0f, 0.0f });

		// Pipeline layout for passing uniform values to shaders
		vk::DescriptorSetLayout descSetLayouts[] = { ResourceManager::getInstance().spritesDescriptorSetLayout, ResourceManager::getInstance().cameraDescriptorSetLayout };
		auto plci = vk::PipelineLayoutCreateInfo()
			.setSetLayoutCount(2)
			.setPSetLayouts(descSetLayouts)
			.setPushConstantRangeCount(0)
			.setPPushConstantRanges(nullptr);

		//device.createPipelineLayout(&plci, nullptr, &pipelineLayout);
		errCheck(device.createPipelineLayout(&plci, nullptr, &pipelineLayout));

		auto gpci = vk::GraphicsPipelineCreateInfo()
			.setStageCount(2)
			.setPStages(shaderStages)
			.setPVertexInputState(&visci)
			.setPInputAssemblyState(&iasci)
			.setPViewportState(&viewportState)
			.setPRasterizationState(&rasterizer)
			.setPMultisampleState(&multisampling)
			.setPDepthStencilState(&depthStencil) // optional
			.setPColorBlendState(&colorBlending)
			.setPDynamicState(nullptr) // optional
			.setLayout(pipelineLayout)
			.setRenderPass(renderPass)
			.setSubpass(0)
			.setBasePipelineHandle(nullptr) // deriving from an existing pipeline
			.setBasePipelineIndex(-1); // optional
		;
		errCheck(device.createGraphicsPipelines(nullptr, 1, &gpci, nullptr, &pipeline));

		/*iasci.topology = vk::PrimitiveTopology::eLineStrip;
		errCheck(device.createPipelineLayout(&plci, nullptr, &pipelineLayoutLines));
		errCheck(device.createGraphicsPipelines(nullptr, 1, &gpci, nullptr, &pipelineLines));*/


		//	Destroy shader modules after graphics pipeline creation
		device.destroyShaderModule(fShaderMod);
		device.destroyShaderModule(vShaderMod);
	}
	void Renderer::destroyGraphicsPipeline()
	{
		device.destroyPipelineLayout(pipelineLayout);
		device.destroyPipeline(pipeline);
	}
	std::vector<char> Renderer::readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}
	void Renderer::createShaderModule(const std::vector<char>& code, vk::ShaderModule & shaderModule)
	{
		auto const smci = vk::ShaderModuleCreateInfo()
			.setCodeSize(code.size())
			.setPCode(reinterpret_cast<const uint32_t*>(code.data()));
		errCheck(device.createShaderModule(&smci, nullptr, &shaderModule));
	}
	void Renderer::createSemaphores()
	{
		auto const si = vk::SemaphoreCreateInfo();
		errCheck(device.createSemaphore(&si, nullptr, &semaphore_Image_Available));
		errCheck(device.createSemaphore(&si, nullptr, &semaphore_Render_Finished));
	}
	void Renderer::destroySemaphores()
	{
		device.destroySemaphore(semaphore_Image_Available);
		device.destroySemaphore(semaphore_Render_Finished);
	}
	void Renderer::createFence()
	{
		auto const fci = vk::FenceCreateInfo().setFlags(vk::FenceCreateFlagBits::eSignaled);
		errCheck(device.createFence(&fci, nullptr, &fence));

	}
	void Renderer::destroyFence()
	{
		device.destroyFence(fence);
	}
#ifdef _DEBUG
	VKAPI_ATTR VkBool32 VKAPI_CALL Renderer::VulkanDebugCallBack(
		VkDebugReportFlagsEXT		flags,
		VkDebugReportObjectTypeEXT	obj_type,
		uint64_t					src_obj,
		size_t						location,
		int32_t						msg_code,
		const char *				layer_prefix,
		const char *				msg,
		void *						user_data
	)
	{
		if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
			LOG("Info report from ");
			LOG(layer_prefix << ":\t");
			LOG(msg << std::endl);
		}
		if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
			LOG("Warning report from ");
			LOG(layer_prefix << ":\t");
			LOG(msg << std::endl);
		}
		if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
			LOG("Performance report from ");
			LOG(layer_prefix << ":\t");
			LOG(msg << std::endl);
		}
		if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
			LOG("Error report from ");
			LOG(layer_prefix << ":\t");
			LOG(msg << std::endl);
		}
		if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
			LOG("Debug report from ");
			LOG(layer_prefix << ":\t");
			LOG(msg << std::endl);
		}
		return false;
	}
	void Renderer::allocateConsole()
	{
		AllocConsole();
		FILE* pCout;
		freopen_s(&pCout, "conin$", "r", stdin);
		freopen_s(&pCout, "conout$", "w", stdout);
		freopen_s(&pCout, "conout$", "w", stderr);
		LOG("<Console Created>\n\n");
	}
	void Renderer::freeConsole()
	{
		system("PAUSE");
		FreeConsole();
	}
	void Renderer::setUpDebugCallback()
	{
		E_vkCreateDebugReportCallbackEXT(instance, &reportInfo, nullptr, &debug_report);
	}
	void Renderer::E_vkCreateDebugReportCallbackEXT(
		vk::Instance & instance,
		vk::DebugReportCallbackCreateInfoEXT * createInfo,
		vk::AllocationCallbacks * allocator,
		vk::DebugReportCallbackEXT * callback)
	{
		auto func = (PFN_vkCreateDebugReportCallbackEXT)instance.getProcAddr("vkCreateDebugReportCallbackEXT");
		if (!func) throw std::runtime_error("Failed to get procedure address for vkCreateDebugReportCallbackEXT");

		func(*reinterpret_cast<VkInstance*>(&instance),
			reinterpret_cast<VkDebugReportCallbackCreateInfoEXT*>(createInfo),
			reinterpret_cast<VkAllocationCallbacks*>(allocator),
			reinterpret_cast<VkDebugReportCallbackEXT*>(callback));
	}
	void Renderer::E_DestroyDebugReportCallbackEXT(vk::Instance & instance, vk::DebugReportCallbackEXT * callback, vk::AllocationCallbacks * allocator)
	{
		auto func = (PFN_vkDestroyDebugReportCallbackEXT)instance.getProcAddr("vkDestroyDebugReportCallbackEXT");
		if (!func) throw std::runtime_error("Failed to get procedure address for DestroyDebugReportCallbackEXT");
		func(*reinterpret_cast<VkInstance*>(&instance),
			*reinterpret_cast<VkDebugReportCallbackEXT*>(callback),
			reinterpret_cast<VkAllocationCallbacks*>(allocator));
	}
	void Renderer::destroyDebugCallback()
	{
		E_DestroyDebugReportCallbackEXT(instance, &debug_report, nullptr);
	}
#else
	void Renderer::allocateConsole() {}
	void Renderer::freeConsole() {}
	void Renderer::setUpDebugCallback() {}
	void Renderer::destroyDebugCallback() {}
#endif
	void SwapchainInfo::querySwapChainSupport(vk::PhysicalDevice gpu, vk::SurfaceKHR surface)
	{
		// 1. surface capabilities
		gpu.getSurfaceCapabilitiesKHR(surface, &capabilities);

		{
			// 2. surface format
			uint32_t formatCount = 0;
			errCheck(gpu.getSurfaceFormatsKHR(surface, &formatCount, nullptr));
			if (formatCount == 0) {
				LOG("Surface formats missing\n");
				exit(-1);
			}
			surfaceFormats.resize(formatCount);
			errCheck(gpu.getSurfaceFormatsKHR(surface, &formatCount, surfaceFormats.data()));
		}
		{
			// 3. presentation mode
			uint32_t presentCount = 0;
			errCheck(gpu.getSurfacePresentModesKHR(surface, &presentCount, nullptr));
			if (presentCount == 0) {
				LOG("Surface formats missing\n");
				exit(-1);
			}
			presentModes.resize(presentCount);
			errCheck(gpu.getSurfacePresentModesKHR(surface, &presentCount, presentModes.data()));

			LOG("*************************************\n          Present Support types\n");
			for (const auto& i : presentModes) {
				if (i == vk::PresentModeKHR::eFifo)
					LOG("eFifo");
				if (i == vk::PresentModeKHR::eFifoRelaxed)
					LOG("eFifoRelaxed");
				if (i == vk::PresentModeKHR::eImmediate)
					LOG("eImmediate");
				if (i == vk::PresentModeKHR::eMailbox)
					LOG("eMailbox");
				if (i == vk::PresentModeKHR::eSharedContinuousRefresh)
					LOG("eSharedContinuousRefresh");
				if (i == vk::PresentModeKHR::eSharedDemandRefresh)
					LOG("eSharedDemandRefresh");
				LOG("\n");
			}
			LOG("*************************************\n");
		}
	}
	vk::SurfaceFormatKHR SwapchainInfo::chooseSwapchainFormat()
	{
		for (const auto& i : surfaceFormats) {
			if (i.format == vk::Format::eB8G8R8A8Unorm &&
				i.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
				return i;
			}
		}
		return surfaceFormats[0];
	}
	vk::PresentModeKHR SwapchainInfo::chooseSwapchainPresentMode()
	{
		for (const auto& i : presentModes) {
			if (i == vk::PresentModeKHR::eImmediate) { // choose one...
				return i;
			}
		}
		return vk::PresentModeKHR::eFifo; //eFifo
	}
	vk::Extent2D SwapchainInfo::chooseSwapchainExtent()
	{
		return capabilities.currentExtent;
	}
	void Renderer::summit()
	{

		//presentQueue.waitIdle();
		//recordCommandBuffers();
		// 1. Acquiring an image from the swapchain
		//(this image is attached in the framebuffer)
		uint32_t imageIndex;
		vk::Result res = device.acquireNextImageKHR(swapchain, UINT64_MAX, semaphore_Image_Available, nullptr, &imageIndex);
		if (res != vk::Result::eSuccess) {
			if (res == vk::Result::eErrorOutOfDateKHR) {
				reInitSwapchain();
				return;
			}
			else if (res != vk::Result::eSuboptimalKHR) {
				exit(-1);
			}
		}
		// 2. Submitting the command buffer to the graphics queue
		vk::Semaphore waitSemaphores[] = { semaphore_Image_Available };
		vk::Semaphore signalSemaphores[] = { semaphore_Render_Finished };
		vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
		auto const si = vk::SubmitInfo()
			.setWaitSemaphoreCount(1)
			.setPWaitSemaphores(waitSemaphores)
			.setPWaitDstStageMask(waitStages)
			.setCommandBufferCount(1)
			.setPCommandBuffers(&commandBuffers[imageIndex])
			.setSignalSemaphoreCount(1)
			.setPSignalSemaphores(signalSemaphores);
		errCheck(graphicsQueue.submit(1, &si, nullptr));

		// 3. Return the image to the swapchain for presentation
		vk::SwapchainKHR swapchains[] = { swapchain };
		auto const pi = vk::PresentInfoKHR()
			.setWaitSemaphoreCount(1)
			.setPWaitSemaphores(signalSemaphores)
			.setSwapchainCount(1)
			.setPSwapchains(swapchains)
			.setPImageIndices(&imageIndex)
			.setPResults(nullptr); //optional
		res = presentQueue.presentKHR(&pi);
		if (res != vk::Result::eSuccess) {
			if (res == vk::Result::eErrorOutOfDateKHR || res == vk::Result::eSuboptimalKHR) {
				reInitSwapchain();
			}
			else {
				exit(-1);
			}
		}
	}
	void Renderer::pushSpritesToBuffers()
	{
		createVertexBuffers();
		createIndexBuffers();

		createUniformBuffers();

		createDescriptorPool();
		createDescriptorSet();

		recordCommandBuffers();
		createSemaphores();
	}
	void Renderer::recordCommandBuffers()
	{
		//	Begin Command Buffer
		//	|	Begin Render Pass
		//	|	|	Bind GraphicsPipeline
		//	|	|	|	Draw
		//	|	End Render Pass
		//	End Command Buffer

		for (size_t i = 0; i < commandBuffers.size(); ++i) {
			auto const beginInfo = vk::CommandBufferBeginInfo()
				.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse)
				.setPInheritanceInfo(nullptr);
			errCheck(commandBuffers[i].begin(&beginInfo));
			// Render Pass
			{
				std::array<vk::ClearValue, 2> clearValues = {};
				clearValues[0].setColor(vk::ClearColorValue().setFloat32({ 0.529f, 0.808f, 0.922f, 1.0f }));
				clearValues[1].setDepthStencil({ 1.0f, 0 });

				auto const renderPassInfo = vk::RenderPassBeginInfo()
					.setRenderPass(renderPass)
					.setFramebuffer(swapchainFrameBuffers[i])
					.setRenderArea({ { 0, 0 }, swapchainExtent })
					.setClearValueCount((uint32_t)clearValues.size())
					.setPClearValues(clearValues.data());

				commandBuffers[i].beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

				commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
				if (Sprite::sprites.size() > 0) {
					// ----------DRAW SPRITES----------
					//binding the vertex buffer
					const vk::DeviceSize offsets[] = { 0 };
					commandBuffers[i].bindVertexBuffers(0, 1, &ResourceManager::getInstance().spritesVertexBuffer, offsets);
					//binding the index buffer
					commandBuffers[i].bindIndexBuffer(ResourceManager::getInstance().spritesIndexBuffer, 0, vk::IndexType::eUint32);

					int32_t vOffset = 0;
					for (auto &sprite : Sprite::sprites) {

						// bind descriptor sets
						const vk::DescriptorSet dSets[] = { sprite->descriptorSet, mainCamera.getDescriptorSet() };
						const uint32_t dOffsets[] = { static_cast<uint32_t>(sprite->uBuffInfo.offset) };
						commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
							pipelineLayout, 0, 2, dSets, 1, dOffsets);

						//drawing indexed
						commandBuffers[i].drawIndexed(6, 1, 0, vOffset, 0); // 6 indices for every 4 vertices in vBuffer (1 rect)
						vOffset += 4;
					}
					// --------------------------------
				}
				//if (Sprite::sprites.size() > 0) {
				//	// ----------DRAW SPRITES----------
				//	//binding the vertex buffer
				//	const vk::DeviceSize offsets[] = { 0 };
				//	commandBuffers[i].bindVertexBuffers(0, 1, &ResourceManager::getInstance().spritesVertexBuffer, offsets);
				//	//binding the index buffer
				//	commandBuffers[i].bindIndexBuffer(ResourceManager::getInstance().spritesIndexBuffer, 0, vk::IndexType::eUint32);

				//	int32_t vOffset = (Sprite::sprites.size() - 1) * 4;
				//	for (auto rit = Sprite::sprites.rbegin(); rit != Sprite::sprites.rend(); ++rit) {

				//		// bind descriptor sets
				//		const vk::DescriptorSet dSets[] = { (*rit)->descriptorSet, mainCamera.getDescriptorSet() };
				//		const uint32_t dOffsets[] = { static_cast<uint32_t>((*rit)->uBuffInfo.offset) };
				//		commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
				//			pipelineLayout, 0, 2, dSets, 1, dOffsets);

				//		//drawing indexed
				//		commandBuffers[i].drawIndexed(6, 1, 0, vOffset, 0); // 6 indices for every 4 vertices in vBuffer (1 rect)
				//		vOffset -= 4;
				//	}
				//	// --------------------------------
				//}

				commandBuffers[i].endRenderPass();
			}
			commandBuffers[i].end();
		}
	}
}