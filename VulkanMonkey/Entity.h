#pragma once
#include "Sprite.h"
#include "include/Box2D/Box2D.h"
#define M2P 60.0f
#define P2M 1/M2P

namespace vm {
	class Entity {
	public:
		static std::vector<Entity*>		drawList;
		static std::vector<Entity>		entities;

		Entity();
		~Entity();

	private:
		Sprite						*sprite;
		glm::mat4					model;
		float						angle;
		float						depth;
		Rect						rect;
		double						timeScale;

	public:
		b2Body						*body;

		void update();
		void draw();
		void destroy();
		void createBody2D(float x, float y);
		void addBoxShape(float width, float height);
		void addCircleShape(float radius, float localX = 0.f, float localY = 0.f);

		bool hasBody() const;
		bool hasSprite() const;
		void setDepth(const float depth);
		float getDepth() const;
		void setTransform(const b2Transform& transform);
		float getAngle() const;

		void setTimeScale(double timeScale);
		double getTimeScale() const;
		

		void setSprite(Sprite* sp);
		Sprite& getSprite();
		glm::mat4& getTranslationMat();
	};
}