#pragma once

struct position {
    float x;
    float y;
};

position operator+(position lhs, position rhs);
position operator-(position lhs, position rhs);
position operator*(position lhs, float rhs);
position operator*(float lhs, position rhs);

struct velocity {
    float dx;
    float dy;
};

struct ai_agent {

};

void init_game(entt::registry& state);

void update(entt::registry& state);

