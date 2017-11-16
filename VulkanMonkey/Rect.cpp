#include "Rect.h"

namespace vm {
	Rect::~Rect() {}

	Rect::Rect() :pos(b2Vec2(0.0f, 0.0f)), size(b2Vec2(0.0f, 0.0f)) {}
	Rect::Rect(b2Vec2 _pos, b2Vec2 _size) : pos(_pos), size(_size) {}

	bool Rect::operator==(Rect const & rhs) const
	{
		return pos == rhs.pos && size == rhs.size;
	}
	Rect::operator bool() const
	{
		return size.x > 0.0f && size.y > 0.0f;
	}
	Rect & Rect::operator= (Rect rhs)
	{
		pos = rhs.pos;
		size = rhs.size;
		return *this;
	}
	Rect & Rect::operator*(const float value)
	{
		pos *= value;
		size *= value;
		return *this;
	}
}