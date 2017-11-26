#include "Entity.h"
#include "ErrorAndLog.h"

namespace vm {
	std::vector <Entity*>	Entity::drawList{};
	Entity::Entity()
	{
		rect = Rect();
		body = nullptr;
		sprite = nullptr;
		depth = 0.0f;
		model = glm::mat4(1.f);
		angle = 0.0f;
		timeScale = 1.f;
	}
	Entity::~Entity()
	{
	}
	void Entity::update()
	{
		if (!sprite) return;
		sprite->setModelPos(model);
		sprite->update();
	}
	void Entity::draw()
	{
		if (!sprite) return;
		Entity::drawList.push_back(this);
	}
	void Entity::setDepth(const float depth)
	{
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, depth - this->depth)); // e.g. new depth -1, old depth 1, then move the z coord by -2 so the z falls in the new depth value (-1)
		this->depth = depth;
	}

	float Entity::getDepth() const
	{
		return depth;
	}

	void Entity::setTransform(const b2Transform& transform)
	{
		angle = transform.q.GetAngle();
		model = glm::translate(glm::mat4(), glm::vec3(transform.p.x * M2P, transform.p.y * M2P, depth));
		model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	float Entity::getAngle() const
	{
		return angle;
	}
	void Entity::setTimeScale(double timeScale)
	{
		this->timeScale = timeScale;
	}
	double Entity::getTimeScale() const
	{
		return timeScale;
	}
	void Entity::createBody2D(float x, float y)
	{
		b2BodyDef bodyDef;
		bodyDef.type = b2BodyType::b2_dynamicBody;
		bodyDef.angle = 0.0f;
		bodyDef.position.Set(x * P2M, y * P2M);
		body = ResourceManager::getInstance().world->CreateBody(&bodyDef);
	}
	void Entity::addBoxShape(float width, float height)
	{
		b2PolygonShape shape;
		shape.SetAsBox(width * P2M, height * P2M);

		b2FixtureDef boxFixtureDef;
		boxFixtureDef.shape = &shape;
		boxFixtureDef.density = 10;
		boxFixtureDef.friction = 1.f;
		boxFixtureDef.restitution = 0.1f;

		body->CreateFixture(&boxFixtureDef);
	}
	void Entity::addCircleShape(float radius, float localX, float localY)
	{
		b2CircleShape shape;
		shape.m_p.Set(localX, localY); //position, relative to body position
		shape.m_radius = radius * P2M; //radius

		b2FixtureDef boxFixtureDef;
		boxFixtureDef.shape = &shape;
		boxFixtureDef.density = 10;
		boxFixtureDef.friction = 1.f;
		boxFixtureDef.restitution = 0.1f;
		body->CreateFixture(&boxFixtureDef);
	}
	void Entity::setSprite(Sprite* sp)
	{
		if (sp) {
			sprite = sp;
			rect = sp->getRect();
		}
	}
	Sprite& Entity::getSprite() 
	{
			return *sprite;
	}
	bool Entity::hasBody() const
	{
		return body != nullptr;
	}
	bool Entity::hasSprite() const
	{
		return sprite != nullptr;
	}
	glm::mat4& Entity::getTranslationMat()
	{ 
		return model;
	}
	void Entity::destroy() {}
}
