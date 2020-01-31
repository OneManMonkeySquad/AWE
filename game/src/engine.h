#pragma once

struct ALLEGRO_DISPLAY;
struct ALLEGRO_EVENT_QUEUE;

struct engine_state {
    ALLEGRO_DISPLAY* display;
    ALLEGRO_EVENT_QUEUE* event_queue;
};

engine_state* create_engine();
void destroy_engine(engine_state * state);