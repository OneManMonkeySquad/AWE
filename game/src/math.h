#pragma once

namespace math
{

template<typename T>
constexpr T lerp(T a, T b, float t)
{
    return a + t * (b - a);
}

struct vector2 {
    float x;
    float y;

    float magnitude() const {
        return sqrt(x * x + y * y);
    }
};

vector2 operator+(vector2 lhs, vector2 rhs);
vector2 operator-(vector2 lhs, vector2 rhs);
vector2 operator*(vector2 lhs, float rhs);
vector2 operator*(float lhs, vector2 rhs);
vector2 operator/(vector2 lhs, float rhs);
vector2& operator+=(vector2& lhs, vector2 rhs);

}