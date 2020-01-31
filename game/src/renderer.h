#pragma once

#include "engine.h"
#include "game.h"

struct camera {
    position position = {0, 0};
    float zoom = 1.0;
    float rotate = 0;
};

struct renderer_state {
    ALLEGRO_FONT* ttf_font;

    camera main_camera;

    ALLEGRO_BITMAP* tiles;
    int tile_map[100 * 100];
};

struct sprite {
    ALLEGRO_BITMAP* bitmap;
};

renderer_state* create_renderer(const engine_state& engine);

entt::registry interpolate_for_rendering(const entt::registry& current, const entt::registry& previous, float a);

void render(const renderer_state& state, const engine_state& engine, const entt::registry& game_state);