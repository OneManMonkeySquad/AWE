
#include "pch.h"
#include "game.h"
#include "renderer.h"
#include "input.h"
#include "ai.h"

position operator+(position l, position r) {
    return { l.x + r.x, l.y + r.y };
}

position operator-(position l, position r) {
    return { l.x - r.x, l.y - r.y };
}

position operator*(position l, float r) {
    return { l.x * r, l.y * r };
}

position operator*(float l, position r) {
    return { l * r.x, l * r.y };
}

position operator/(position l, float r) {
    return { l.x / r, l.y / r };
}

position& operator+=(position& l, position r) {
    l.x += r.x;
    l.y += r.y;
    return l;
}

entt::registry create_game() {
    entt::registry state;

    init_ai(state);

    auto enemy = al_load_bitmap("data/enemy.png");
    for (auto i = 0; i < 10; ++i) {
        auto entity = state.create();
        state.assign<position>(entity, float(rand() % 1500), float(rand() % 1500));
        state.assign<sprite>(entity, enemy);
        state.assign<ai_agent>(entity);
    }

    auto deer = al_load_bitmap("data/deer.png");
    for (auto i = 0; i < 10; ++i) {
        auto entity = state.create();
        state.assign<position>(entity, float(rand() % 1500), float(rand() % 1500));
        state.assign<sprite>(entity, deer);
        state.assign<velocity>(entity, ((rand() * 12345 + 46789) % 30) * 0.35f, ((rand() * 546788784563 + 123456) % 30) * .25f);
    }

    auto tree_bmp = al_load_bitmap("data/tree.png");
    for (auto i = 0; i < 60; ++i) {
        auto entity = state.create();
        state.assign<position>(entity, float(rand() % 1500), float(rand() % 1500));
        state.assign<sprite>(entity, tree_bmp);
        state.assign<tree>(entity);
    }

    camera main_camera{};
    main_camera.position = { 0, 0 };
    main_camera.rotate = 0;
    state.set<camera>() = main_camera;

    return state;
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

void update(entt::registry& state) {
    update_physics(state);
    update_ai(state);
}

void update_camera(entt::registry& state, float delta_time_ms) {
    auto& cam = state.ctx<camera>();

    auto inp = state.ctx<input>();
    cam.zoom = std::clamp(cam.zoom + inp.wheel * delta_time_ms * 0.05f, 0.4f, 1.8f);
    cam.position.x += (inp.d - inp.a) * delta_time_ms * cam.zoom * 0.5f;
    cam.position.y += (inp.s - inp.w) * delta_time_ms * cam.zoom * 0.5f;
}