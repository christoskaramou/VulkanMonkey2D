#pragma once
#include "glm_.h"
#include "Vulkan_.h"
#include <array>
#include <unordered_set>



namespace vm {
	struct Vertex {
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 texCoord;
		bool operator==(const Vertex& other) const {
			return pos == other.pos && color == other.color && texCoord == other.texCoord;
		}
		static vk::VertexInputBindingDescription getBindingDescription() {
			auto const bindDescription = vk::VertexInputBindingDescription()
				.setBinding(0) //index of the binding in the array of bindings
				.setStride(sizeof(Vertex))
				.setInputRate(vk::VertexInputRate::eVertex);
			return bindDescription;
		}
		static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescription() {
			std::array<vk::VertexInputAttributeDescription, 3> const attributeDescriptions = {
				vk::VertexInputAttributeDescription()
				.setBinding(0) //index of the binding to get per-vertex data
				.setLocation(0) //location directive of the input in the vertex shader
				.setFormat(vk::Format::eR32G32B32Sfloat) //vec3
				.setOffset(offsetof(Vertex, pos)),
				vk::VertexInputAttributeDescription()
				.setBinding(0)
				.setLocation(1)
				.setFormat(vk::Format::eR32G32B32Sfloat) //vec3
				.setOffset(offsetof(Vertex, color)),
				vk::VertexInputAttributeDescription()
				.setBinding(0)
				.setLocation(2)
				.setFormat(vk::Format::eR32G32Sfloat) //vec2
				.setOffset(offsetof(Vertex, texCoord))
			};
			return attributeDescriptions;
		}
	};
}
template<> struct std::hash<vm::Vertex> {
	size_t operator()(vm::Vertex const& vertex) const {
		return ((hash<glm::vec3>()(vertex.pos) ^
			(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
			(hash<glm::vec2>()(vertex.texCoord) << 1);
	}
};