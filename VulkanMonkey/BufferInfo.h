#pragma once
#include "Vulkan_.h"
namespace vm {
	struct BufferInfo {
		vk::DeviceSize offset;
		vk::BufferUsageFlagBits usage;
		vk::DeviceSize size;
	};
}