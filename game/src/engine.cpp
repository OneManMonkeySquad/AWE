
#include "pch.h"
#include "engine.h"
#include "utils.h"

engine_state* create_engine() {
    auto new_engine = new engine_state;

    al_set_new_display_flags(ALLEGRO_WINDOWED);
    new_engine->display = al_create_display(1280, 1024);
    al_set_window_title(new_engine->display, "Awe");

    new_engine->event_queue = al_create_event_queue();
    al_register_event_source(new_engine->event_queue, al_get_display_event_source(new_engine->display));
    al_register_event_source(new_engine->event_queue, al_get_keyboard_event_source());
    al_register_event_source(new_engine->event_queue, al_get_mouse_event_source());

    return new_engine;
}

void destroy_engine(engine_state * state) {
    al_destroy_display(state->display);
    al_destroy_event_queue(state->event_queue);
    delete state;
}