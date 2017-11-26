#include "Vulkan_.h"
#include "ErrorAndLog.h"

namespace vm {
	// helper vk functions
	uint32_t Helper::findMemoryType(vk::PhysicalDevice gpu, uint32_t typeFilter, vk::MemoryPropertyFlags properties) const
	{
		vk::PhysicalDeviceMemoryProperties memProperties;
		memProperties = gpu.getMemoryProperties();

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
			if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}
		exit(-1); // no suitable memory type
	}
	void Helper::createBuffer(vk::PhysicalDevice &gpu, vk::Device &device, vk::DeviceSize &size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer & buffer, vk::DeviceMemory & bufferMemory) const
	{
		//create buffer (GPU buffer)
		auto const bufferInfo = vk::BufferCreateInfo()
			.setSize(size)
			.setUsage(usage)
			.setSharingMode(vk::SharingMode::eExclusive); // used by graphics queue only

		errCheck(device.createBuffer(&bufferInfo, nullptr, &buffer));

		//retrieve memory requirement of buffer
		vk::MemoryRequirements memRequirements;
		device.getBufferMemoryRequirements(buffer, &memRequirements);

		//allocate memory of buffer
		auto const allocInfo = vk::MemoryAllocateInfo()
			.setAllocationSize(memRequirements.size)
			.setMemoryTypeIndex(findMemoryType(gpu, memRequirements.memoryTypeBits, properties));
		errCheck(device.allocateMemory(&allocInfo, nullptr, &bufferMemory));

		//binding memory with buffer
		errCheck(device.bindBufferMemory(buffer, bufferMemory, 0));
	}
	void Helper::destroyBuffer(vk::Device &device, vk::Buffer &buffer, vk::DeviceMemory &bufferMemory) const
	{
		device.destroyBuffer(buffer);
		device.freeMemory(bufferMemory);
	}
	void Helper::copyBuffer(vk::Device &device, vk::CommandPool &cmdPool, vk::Queue &queue, vk::Buffer *srcBuffer, vk::Buffer *dstBuffer, vk::DeviceSize *size) const
	{
		vk::CommandBuffer copyCmd;

		auto const cbai = vk::CommandBufferAllocateInfo()
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandPool(cmdPool)
			.setCommandBufferCount(1);
		errCheck(device.allocateCommandBuffers(&cbai, &copyCmd));

		auto const cbbi = vk::CommandBufferBeginInfo();
		errCheck(copyCmd.begin(&cbbi));

		vk::BufferCopy bufferCopy{};
		bufferCopy.size = *size;

		copyCmd.copyBuffer(*srcBuffer, *dstBuffer, 1, &bufferCopy);

		errCheck(copyCmd.end());

		auto const si = vk::SubmitInfo()
			.setCommandBufferCount(1)
			.setPCommandBuffers(&copyCmd);
		errCheck(queue.submit(1, &si, nullptr));
		errCheck(queue.waitIdle());

		device.freeCommandBuffers(cmdPool, 1, &copyCmd);
	}
	void Helper::createImage(vk::PhysicalDevice gpu, vk::Device device, uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image & image, vk::DeviceMemory & imageMemory) const
	{
		auto const imageInfo = vk::ImageCreateInfo()
			.setImageType(vk::ImageType::e2D)
			.setFormat(format)
			.setExtent({ width, height, 1 })
			.setMipLevels(1)
			.setArrayLayers(1)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(tiling)
			.setUsage(usage)
			.setSharingMode(vk::SharingMode::eExclusive)
			.setInitialLayout(vk::ImageLayout::ePreinitialized);

		errCheck(device.createImage(&imageInfo, nullptr, &image));

		auto const memRequirements = device.getImageMemoryRequirements(image);

		auto const allocInfo = vk::MemoryAllocateInfo()
			.setAllocationSize(memRequirements.size)
			.setMemoryTypeIndex(findMemoryType(gpu, memRequirements.memoryTypeBits, properties));

		errCheck(device.allocateMemory(&allocInfo, nullptr, &imageMemory));

		errCheck(device.bindImageMemory(image, imageMemory, 0));
	}
	void Helper::destroyImage(vk::Device device, vk::Image image, vk::DeviceMemory bufferMemory) const
	{
		device.destroyImage(image);
		device.freeMemory(bufferMemory);
	}
	vk::CommandBuffer Helper::beginSingleCommandBuffer(vk::Device device, vk::CommandPool cmdPool) const
	{
		auto const allocInfo = vk::CommandBufferAllocateInfo()
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(1)
			.setCommandPool(cmdPool);

		vk::CommandBuffer commandBuffer;
		errCheck(device.allocateCommandBuffers(&allocInfo, &commandBuffer));

		auto const beginInfo = vk::CommandBufferBeginInfo()
			.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

		errCheck(commandBuffer.begin(&beginInfo));

		return commandBuffer;
	}
	void Helper::endSingleCommandBuffer(vk::Device device, vk::CommandPool cmdPool, vk::Queue queue, vk::CommandBuffer commandBuffer) const
	{
		errCheck(commandBuffer.end());

		auto const submitInfo = vk::SubmitInfo()
			.setCommandBufferCount(1)
			.setPCommandBuffers(&commandBuffer);

		errCheck(queue.submit(1, &submitInfo, nullptr));
		errCheck(queue.waitIdle());

		device.freeCommandBuffers(cmdPool, 1, &commandBuffer);
	}
	void Helper::copyImage(vk::Device device, vk::CommandPool cmdPool, vk::Queue queue, vk::Image srcImage, vk::Image dstImage, uint32_t width, uint32_t height) const
	{
		vk::CommandBuffer commandBuffer = beginSingleCommandBuffer(device, cmdPool);

		auto const subResource = vk::ImageSubresourceLayers()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseArrayLayer(0)
			.setMipLevel(0)
			.setLayerCount(1);
		auto const region = vk::ImageCopy()
			.setSrcSubresource(subResource)
			.setDstSubresource(subResource)
			.setSrcOffset({ 0,0,0 })
			.setDstOffset({ 0,0,0 })
			.setExtent({ width, height, 1 });

		commandBuffer.copyImage(srcImage, vk::ImageLayout::eTransferSrcOptimal,
			dstImage, vk::ImageLayout::eTransferDstOptimal, 1, &region);

		endSingleCommandBuffer(device, cmdPool, queue, commandBuffer);
	}
	bool Helper::hasStencilCompoment(vk::Format format) const
	{
		return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
	}
	void Helper::transitionImageLayout(vk::Device device, vk::CommandPool cmdPool, vk::Queue queue, vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::Format format) const
	{
		vk::CommandBuffer commandBuffer = beginSingleCommandBuffer(device, cmdPool);

		auto barrier = vk::ImageMemoryBarrier()
			.setOldLayout(oldLayout)
			.setNewLayout(newLayout)
			.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setImage(image);
		if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
			barrier.setSubresourceRange({ vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 });
			if (hasStencilCompoment(format)) {
				barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
			}
		}
		else
			barrier.setSubresourceRange({ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
		if (oldLayout == vk::ImageLayout::ePreinitialized && newLayout == vk::ImageLayout::eTransferSrcOptimal) {
			barrier
				.setSrcAccessMask(vk::AccessFlagBits::eHostWrite)
				.setDstAccessMask(vk::AccessFlagBits::eTransferRead);
		}
		else if (oldLayout == vk::ImageLayout::ePreinitialized && newLayout == vk::ImageLayout::eTransferDstOptimal) {
			barrier
				.setSrcAccessMask(vk::AccessFlagBits::eHostWrite)
				.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
		}
		else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
			barrier
				.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
				.setDstAccessMask(vk::AccessFlagBits::eShaderRead); // | vk::AccessFlagBits::eInputAttachmentRead);
		}
		else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
			barrier
				.setSrcAccessMask(vk::AccessFlags())
				.setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite);
		}
		else {
			exit(-1);
		}

		commandBuffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTopOfPipe,
			vk::DependencyFlags(),
			0, nullptr,
			0, nullptr,
			1, &barrier);

		endSingleCommandBuffer(device, cmdPool, queue, commandBuffer);
	}
	void Helper::createImageView(vk::Device device, vk::Image image, vk::Format format, vk::ImageView &imageView, vk::ImageAspectFlags aspectFlags) const
	{
		auto const viewInfo = vk::ImageViewCreateInfo()
			.setImage(image)
			.setViewType(vk::ImageViewType::e2D)
			.setFormat(format)
			.setSubresourceRange({ aspectFlags, 0, 1, 0, 1 });

		errCheck(device.createImageView(&viewInfo, nullptr, &imageView));
	}
	void Helper::copyBufferToImage(vk::Device device, vk::CommandPool cmdPool, vk::Queue queue, vk::Buffer buffer, vk::Image image, int x, int y, int width, int height) const
	{
		vk::CommandBuffer commandBuffer = beginSingleCommandBuffer(device, cmdPool);

		vk::BufferImageCopy region;
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { x, y, 0 };
		region.imageExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };

		commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);

		endSingleCommandBuffer(device, cmdPool, queue, commandBuffer);
	}
	vk::Format Helper::findSupportedFormat(vk::PhysicalDevice gpu, const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) const
	{
		for (auto format : candidates) {
			vk::FormatProperties props = gpu.getFormatProperties(format);
			if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}

		exit(-1); // if not found supported format
	}
	vk::Format Helper::findDepthFormat(vk::PhysicalDevice gpu)
	{
		return findSupportedFormat(gpu,
			{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
			vk::ImageTiling::eOptimal,
			vk::FormatFeatureFlagBits::eDepthStencilAttachment);
	}
}