#pragma once

#include "renderer.h"

struct ALLEGRO_EVENT_QUEUE;
struct renderer_state;

struct engine_state {
    ALLEGRO_EVENT_QUEUE* event_queue;

    renderer_state* renderer;
};

engine_state* create_engine();
void destroy_engine(engine_state * state);