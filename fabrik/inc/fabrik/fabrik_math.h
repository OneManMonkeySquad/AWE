
#pragma once

namespace math {
	constexpr float degrees_to_rad = ALLEGRO_PI / 180.0;

	template<typename T>
	constexpr T clamp(T val, T min, T max) {
		if (val < min)
			return min;

		if (val > max)
			return max;

		return val;
	}

	template<typename T>
	constexpr T lerp(T a, T b, float t) {
		return a + t * (b - a);
	}


	struct vector2 {
		float x, y;

		float magnitude() const {
			return sqrt(x * x + y * y);
		}
	};

	template<typename Archive>
	void serialize(Archive& archive, vector2& vec) {
		archive(vec.x, vec.y);
	}

	vector2 operator+(vector2 lhs, vector2 rhs);
	vector2 operator-(vector2 lhs, vector2 rhs);
	vector2 operator*(vector2 lhs, float rhs);
	vector2 operator*(float lhs, vector2 rhs);
	vector2 operator/(vector2 lhs, float rhs);
	vector2& operator+=(vector2& lhs, vector2 rhs);


	struct color {
		uint8_t r, g, b, a;

		static color red, white, green;
	};
}