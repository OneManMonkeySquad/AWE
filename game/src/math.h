#pragma once

namespace math
{

template<typename T>
constexpr T lerp(T a, T b, float t)
{
    return a + t * (b - a);
}

}