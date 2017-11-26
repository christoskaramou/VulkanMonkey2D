#pragma once
#include "include/Box2D/Box2D.h"
namespace vm {
	class Rect
	{
	public:
		Rect();
		Rect(b2Vec2 posistion, b2Vec2 size);
		~Rect();
	public:
		b2Vec2 pos;
		b2Vec2 size;

		bool operator==(Rect const & rhs) const;
		operator bool() const;
		Rect & operator= (Rect rhs);
		Rect& operator* (const float value);
	};
}