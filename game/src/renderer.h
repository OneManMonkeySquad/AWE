#pragma once

#include "engine.h"
#include "game.h"

struct engine_state;

struct camera {
    position position = {0, 0};
    float zoom = 1.0;
    float rotate = 0;
};

struct renderer_state {
    ALLEGRO_DISPLAY* display;

    ALLEGRO_FONT* ttf_font;

    ALLEGRO_BITMAP* tiles;
    int tile_map[100 * 100];
};

struct sprite {
    ALLEGRO_BITMAP* bitmap;
};

renderer_state* create_renderer();
void destroy_renderer(renderer_state* state);

entt::registry interpolate_for_rendering(const entt::registry& current, const entt::registry& previous, float a);

void render(const engine_state* engine, const camera cam, const entt::registry& game_state);