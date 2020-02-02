
#include "pch.h"
#include "engine.h"
#include "utils.h"

engine_state* create_engine() {
    if (!al_init())
        panic();

    if (!al_init_primitives_addon())
        panic();

    if (!al_init_image_addon())
        panic();

    if (!al_init_ttf_addon())
        panic();

    if (!al_install_mouse())
        panic();

    if (!al_install_keyboard())
        panic();

    auto new_engine = new engine_state;

    new_engine->renderer = create_renderer();

    new_engine->event_queue = al_create_event_queue();
    al_register_event_source(new_engine->event_queue, al_get_display_event_source(new_engine->renderer->display));
    al_register_event_source(new_engine->event_queue, al_get_keyboard_event_source());
    al_register_event_source(new_engine->event_queue, al_get_mouse_event_source());

    return new_engine;
}

void destroy_engine(engine_state * state) {
    destroy_renderer(state->renderer);
    al_destroy_event_queue(state->event_queue);
    delete state;
}