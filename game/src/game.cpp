
#include "pch.h"
#include "game.h"
#include "renderer.h"
#include "input.h"
#include "ai.h"
#include "utils.h"





entt::registry create_game() {
    entt::registry state;

    init_ai(state);

    auto enemy = al_load_bitmap("data/enemy.png");
    for (auto i = 0; i < 1; ++i) {
        auto entity = state.create();
        state.assign<position>(entity, float(rand() % 1500), float(rand() % 1500));
        state.assign<sprite>(entity, enemy);
        state.assign<ai_agent>(entity);
        state.assign<inventory>(entity);
    }

    auto deer_bmp = al_load_bitmap("data/deer.png");
    for (auto i = 0; i < 10; ++i) {
        auto entity = state.create();
        state.assign<position>(entity, float(rand() % 1500), float(rand() % 1500));
        state.assign<sprite>(entity, deer_bmp);
        state.assign<velocity>(entity, ((rand() * 12345 + 46789) % 30) * 0.25f, ((rand() * 546788784563 + 123456) % 30) * .15f);
        state.assign<deer>(entity);
    }

    auto axe_bmp = al_load_bitmap("data/axe.png");
    for (auto i = 0; i < 10; ++i) {
        auto entity = state.create();
        state.assign<position>(entity, float(rand() % 1500), float(rand() % 1500));
        state.assign<sprite>(entity, axe_bmp);
        state.assign<item>(entity, item_type::axe);
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
        velocity.dx *= 0.9f;
        velocity.dy *= 0.9f;
    });
}


void update(entt::registry& state) {
    update_utils(state);
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


void kill(entt::registry& state, entt::entity target) {
    if (state.has<deer>(target)) {
        auto tree_stump = al_load_bitmap("data/dead_deer.png");
        state.remove<velocity>(target);
        state.replace<sprite>(target, tree_stump);
        state.remove<deer>(target);

        auto sp = al_load_bitmap("data/meat.png");
        auto pos = state.get<position>(target);
        for (int i = 0; i < 3; ++i) {
            auto new_meat = state.create();
            state.assign<position>(new_meat, pos);
            state.assign<sprite>(new_meat, sp);
            state.assign<item>(new_meat, item_type::meat);
            state.assign<velocity>(new_meat, 0.4f * (rand() % 20 - 10), 0.4f * (rand() % 20 - 10));
        }
    }
    else if (state.has<tree>(target)) {
        auto tree_stump = al_load_bitmap("data/tree_stump.png");
        state.replace<sprite>(target, tree_stump);
        state.remove<tree>(target);

        auto sp = al_load_bitmap("data/wood.png");
        auto pos = state.get<position>(target);
        for (int i = 0; i < 3; ++i) {
            auto new_wood = state.create();
            state.assign<position>(new_wood, pos);
            state.assign<sprite>(new_wood, sp);
            state.assign<item>(new_wood, item_type::wood);
            state.assign<velocity>(new_wood, 0.8f * (rand() % 20 - 10), 0.8f * (rand() % 20 - 10));
        }
    }
    else {
        state.destroy(target);
    }
}


bool inventory_add_item(entt::registry& state, entt::entity inventory_owner, entt::entity new_item) {
    assert(state.has<item>(new_item));

    auto& inv = state.get<inventory>(inventory_owner);

    for (auto i = 0; i < inv.items.size(); ++i) {
        if (inv.items[i] == entt::null) {
            inv.items[i] = new_item;

            debug_print("added item at %d", i);

            if (state.has<position>(new_item)) {
                state.remove<position>(new_item);
            }

            return true;
        }
    }
    return false;
}


bool inventory_has_item_of_type(entt::registry& state, entt::entity inventory_owner, item_type type) {
    auto& inv = state.get<inventory>(inventory_owner);

    for (auto i = 0; i < inv.items.size(); ++i) {
        if (inv.items[i] == entt::null)
            return false;

        auto it = state.get<item>(inv.items[i]);
        if (it.type == type)
            return true;
    }
    return false;
}


entt::entity inventory_remove_item_of_type(entt::registry& state, entt::entity inventory_owner, item_type type) {
    auto& inv = state.get<inventory>(inventory_owner);

    for (auto i = 0; i < inv.items.size(); ++i) {
        if (inv.items[i] == entt::null)
            return entt::null;

        auto it = state.get<item>(inv.items[i]);
        if (it.type == type) {
            auto item = inv.items[i];
            inv.items[i] = entt::null;
            return item;
        }
    }
    return entt::null;
}