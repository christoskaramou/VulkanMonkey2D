#pragma once
#include "Sprite.h"
#include "Box2D\Box2D.h"
#define M2P 32.0f
#define P2M 1/M2P

namespace vm {
	class Entity {
	public:
		static std::vector<std::shared_ptr<Entity>>		entities;

		Entity();
		Entity(std::shared_ptr<Sprite> sp);
		~Entity();

	private:
		std::shared_ptr<Sprite>		sprite;
		glm::mat4					model;
		float						angle;
		float						depth;
		Rect						rect;

	public:
		b2Body						*body;

		void update();
		void draw();
		void destroy();
		void createBody2D(float x, float y);
		void addBoxShape(float width, float height);
		void addCircleShape(float radius, float localX = 0.f, float localY = 0.f);

		bool hasBody();
		void setDepth(const float depth);
		void setTransform(const b2Transform& transform);
		float getAngle();

		void setSprite(std::shared_ptr<Sprite> sp);
		Sprite& getSprite();
		glm::mat4& getTranslationMat();
	};
}