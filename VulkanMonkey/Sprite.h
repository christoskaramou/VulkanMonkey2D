#pragma once
#include "ResourceManager.h"
#include "glm_.h"
#include "BufferInfo.h"

namespace vm {
	enum class SpriteType {
		//None,
		//Entity,
		userDefinedRect,
		userDefinedRectStatic
	};

	struct UniformBufferObject {
		glm::mat4 model;
	};

	class Sprite
	{
		friend class Renderer;
	public:
		static std::vector<Sprite*>			sprites;

		BufferInfo						vBuffInfo;
		BufferInfo						iBuffInfo;
		BufferInfo						uBuffInfo;
		UniformBufferObject				ubo{};
		std::vector<Vertex>				vertices;
		std::vector<uint32_t>			indices;
		vk::DescriptorSet				descriptorSet;
		Texture							texture;
		bool							needsUpdate;		//this sprite needs to be updated (changes to uniform buffer mem)

		Sprite(Rect definedRect, std::string textureName = "");
		Rect getRect();
		void setTexture(std::string path);
		void setModelPos(glm::mat4 modelPos);
		bool isMapped() const;
		SpriteType getSpriteType() const;
		unsigned int getSpriteID() const;
		void update();

	private:
		void							*_uniformMemory;				// pointer to uniform data memory
		SpriteType						type;
		bool							isSpriteMapped;
		unsigned int					spriteID;
		Helper							helper;
		Rect							rect;
		void createDescriptorSet(const vk::DescriptorPool &descriptorPool, const Texture &texture);
		// only for shared uniform buffers to speed up things
		void setPointerToUniformBufferMem(vk::DeviceMemory& uniformBufferMem);
	};
}