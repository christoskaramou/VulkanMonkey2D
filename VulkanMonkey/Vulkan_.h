#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>
#include <vulkan/vk_sdk_platform.h>
namespace vm {
	class Helper {
	public:
		// helper vk functions
		uint32_t findMemoryType(vk::PhysicalDevice gpu, uint32_t typeFilter, vk::MemoryPropertyFlags properties);
		void createBuffer(vk::PhysicalDevice &gpu, vk::Device &device, vk::DeviceSize &size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer & buffer, vk::DeviceMemory & bufferMemory);
		void destroyBuffer(vk::Device &device, vk::Buffer &buffer, vk::DeviceMemory &bufferMemory);
		void copyBuffer(vk::Device &device, vk::CommandPool &cmdPool, vk::Queue &queue, vk::Buffer *srcBuffer, vk::Buffer *dstBuffer, vk::DeviceSize *size);
		void createImage(vk::PhysicalDevice gpu, vk::Device device, uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image & image, vk::DeviceMemory & imageMemory);
		void destroyImage(vk::Device device, vk::Image image, vk::DeviceMemory bufferMemory);
		vk::CommandBuffer beginSingleCommandBuffer(vk::Device device, vk::CommandPool cmdPool);
		void endSingleCommandBuffer(vk::Device device, vk::CommandPool cmdPool, vk::Queue queue, vk::CommandBuffer commandBuffer);
		void copyImage(vk::Device device, vk::CommandPool cmdPool, vk::Queue queue, vk::Image srcImage, vk::Image dstImage, uint32_t width, uint32_t height);
		bool hasStencilCompoment(vk::Format format);
		void transitionImageLayout(vk::Device device, vk::CommandPool cmdPool, vk::Queue queue, vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::Format format = vk::Format::eUndefined);
		void createImageView(vk::Device device, vk::Image image, vk::Format format, vk::ImageView &imageView, vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor);
		void copyBufferToImage(vk::Device device, vk::CommandPool cmdPool, vk::Queue queue, vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);
		vk::Format findSupportedFormat(vk::PhysicalDevice gpu, const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
		vk::Format findDepthFormat(vk::PhysicalDevice gpu);
	};
}