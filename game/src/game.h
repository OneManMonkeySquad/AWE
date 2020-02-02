#pragma once

struct position {
    float x;
    float y;

    float magnitude() const {
        return sqrt(x * x + y * y);
    }
};

position operator+(position lhs, position rhs);
position operator-(position lhs, position rhs);
position operator*(position lhs, float rhs);
position operator*(float lhs, position rhs);
position operator/(position lhs, float rhs);
position& operator+=(position& lhs, position rhs);

struct velocity {
    float dx;
    float dy;
};

using tree = entt::tag<"tree"_hs>;

entt::registry create_game();

void update(entt::registry& state);
void update_camera(entt::registry& state, float delta_time_ms);