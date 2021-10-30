
#pragma once

namespace math {
	constexpr float degrees_to_rad = ALLEGRO_PI / 180.0;
	constexpr float rad_to_degrees = 180.0 / ALLEGRO_PI;

	template<typename T>
	constexpr T lerp(T a, T b, float t) {
		return a + t * (b - a);
	}


	struct vector2 {
		float x = 0, y = 0;

		constexpr float sqr_magnitude() const noexcept { return x * x + y * y; }
		float magnitude() const noexcept { return std::sqrt(x * x + y * y); }
		constexpr float dot(const vector2& other) const { return x * other.x + y * other.y; }
		vector2 normalised() const;

		vector2 rotate(float angle) const noexcept {
			const auto cos = std::cos(angle * degrees_to_rad);
			const auto sin = std::sin(angle * degrees_to_rad);
			vector2 result;
			result.x = cos * x - sin * y;
			result.y = sin * x + cos * y;
			return result;
		}

		/// [0,360] Angle of unit vector.
		float angle() const noexcept {
			return std::atan2(y, x) * rad_to_degrees;
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

		static color red, white, green, blue;
	};
}