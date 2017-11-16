#pragma once
#include "Vulkan_.h"

namespace vm {
	struct Texture {
		vk::Image				image;
		vk::DeviceMemory		imageMem;
		vk::ImageView			imageView;
		std::string				name;
	};
}

