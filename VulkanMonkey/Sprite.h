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
		friend class Renderer; // access privates
	public:
		static std::vector<Sprite*>			sprites;


		Sprite(Rect _rect, std::vector<std::string> imagePathNames = {""});
		Rect getRect() const;
		void setModelPos(glm::mat4 modelPos);
		bool isMapped() const;
		SpriteType getSpriteType() const;
		unsigned int getSpriteID() const;

		// a dSet also contains the imageView data of a texture, assign an other one in a dynamic cmdBuffer can change the texture of the sprite
		void setActiveDescriptorSet(unsigned int num);
		void acquireNextImage(uint32_t start, uint32_t end);

		void update();

	private:
		vk::DescriptorSet				*descriptorSet;		// the active descriptorSet pointer from the list
		std::vector<vk::DescriptorSet>	descriptorSets{};
		std::vector<Texture>			textures{};
		void							*_uniformMemory;	// pointer to uniform data memory
		UniformBufferObject				ubo{};
		SpriteType						type;
		bool							isSpriteMapped;
		unsigned int					spriteID;
		Helper							helper;
		Rect							rect;
		bool							needsUpdate;		//this sprite needs to be updated (changes to uniform buffer mem)

		std::vector<Vertex>				vertices;
		std::vector<uint32_t>			indices;

		BufferInfo						vBuffInfo;
		BufferInfo						iBuffInfo;
		BufferInfo						uBuffInfo;

		void setTextures(const std::vector<std::string>& imagePathNames);
		void setTextures(const std::vector<Texture>& textures);
		Texture createNewTexture(std::string imagePath);

		void createDescriptorSets(const vk::DescriptorPool &descriptorPool);
		// only for shared uniform buffers to speed up things
		void setPointerToUniformBufferMem(vk::DeviceMemory& uniformBufferMem);
	};
}