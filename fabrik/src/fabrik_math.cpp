#include "pch.h"
#include "fabrik_math.h"

namespace math {
	vector2 operator+(vector2 l, vector2 r) {
		return { l.x + r.x, l.y + r.y };
	}

	vector2 operator-(vector2 l, vector2 r) {
		return { l.x - r.x, l.y - r.y };
	}

	vector2 operator*(vector2 l, float r) {
		return { l.x * r, l.y * r };
	}

	vector2 operator*(float l, vector2 r) {
		return { l * r.x, l * r.y };
	}

	vector2 operator/(vector2 l, float r) {
		return { l.x / r, l.y / r };
	}

	vector2& operator+=(vector2& l, vector2 r) {
		l.x += r.x;
		l.y += r.y;
		return l;
	}

	color color::red{ 255, 0, 0, 255 };
	color color::white{ 255, 255, 255, 255 };
	color color::green{ 0, 255, 0, 255 };
	color color::blue{ 0, 0, 255, 255 };
}