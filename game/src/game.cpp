
#include "pch.h"
#include "game.h"
#include "renderer.h"

position operator+(position lhs, position rhs) {
    return { lhs.x + rhs.x, lhs.y + rhs.y };
}

position operator-(position lhs, position rhs) {
    return { lhs.x - rhs.x, lhs.y - rhs.y };
}

position operator*(position lhs, float rhs) {
    return { lhs.x * rhs, lhs.y * rhs };
}

position operator*(float lhs, position rhs) {
    return { lhs * rhs.x, lhs * rhs.y };
}

void init_game(entt::registry& state) {
    auto enemy = al_load_bitmap("data/enemy.png");
    for (auto i = 0; i < 10; ++i) {
        auto entity = state.create();
        state.assign<position>(entity, (rand() % 1500) * 10.f, (rand() % 1500) * 10.f);
        state.assign<sprite>(entity, enemy);
        state.assign<velocity>(entity, ((rand() * 12345 + 46789) % 30) * 0.35f, ((rand() * 546788784563 + 123456) % 30) * .25f);
    }

    auto deer = al_load_bitmap("data/deer.png");
    for (auto i = 0; i < 10; ++i) {
        auto entity = state.create();
        state.assign<position>(entity, (rand() % 1500) * 10.f, (rand() % 1500) * 10.f);
        state.assign<sprite>(entity, deer);
        state.assign<velocity>(entity, ((rand() * 12345 + 46789) % 30) * 0.35f, ((rand() * 546788784563 + 123456) % 30) * .25f);
    }

    auto tree = al_load_bitmap("data/tree.png");
    for (auto i = 0; i < 60; ++i) {
        auto entity = state.create();
        state.assign<position>(entity, (float)(rand() % 1400), (float)(rand() % 1400));
        state.assign<sprite>(entity, tree);
    }
}

namespace
{
void update_ai(entt::registry& state) {
    
}

void update_physics(entt::registry& state) {
    state.group<position, velocity>().each([](auto& pos, auto& vel) {
        pos.x += vel.dx;
        pos.y += vel.dy;
        if (pos.x < 0) { pos.x = 0; vel.dx *= -1; }
        if (pos.x > 1400) { pos.x = 1400; vel.dx *= -1; }
        if (pos.y < 0) { pos.y = 0; vel.dy *= -1; }
        if (pos.y > 1400) { pos.y = 1400; vel.dy *= -1; }
    });

    state.view<velocity>().each([](auto& velocity) {
        velocity.dx *= 0.999f;
        velocity.dy *= 0.999f;
    });
}
}

void update(entt::registry& state) {
    update_physics(state);
}

