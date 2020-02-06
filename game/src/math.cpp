#include "pch.h"
#include "math.h"

namespace math
{

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

}