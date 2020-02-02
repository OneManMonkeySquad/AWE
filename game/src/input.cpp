
#include "pch.h"
#include "input.h"
#include "engine.h"

bool handle_input(engine_state& engine, entt::registry& state) {
    ALLEGRO_KEYBOARD_STATE keyboard_state;
    al_get_keyboard_state(&keyboard_state);

    input new_input;
    new_input.w = al_key_down(&keyboard_state, ALLEGRO_KEY_W);
    new_input.s = al_key_down(&keyboard_state, ALLEGRO_KEY_S);
    new_input.a = al_key_down(&keyboard_state, ALLEGRO_KEY_A);
    new_input.d = al_key_down(&keyboard_state, ALLEGRO_KEY_D);

    new_input.wheel = 0;

    ALLEGRO_EVENT ev;
    while (al_get_next_event(engine.event_queue, &ev)) {
        switch (ev.type) {
        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            return true; // quit game

        case ALLEGRO_EVENT_KEY_DOWN:
            switch (ev.keyboard.keycode) {
            case ALLEGRO_KEY_ESCAPE:
                return true; // quit game
            }
            break;

        case ALLEGRO_EVENT_MOUSE_AXES:
            new_input.wheel = ev.mouse.dz;
            break;
        }
    }

    state.set<input>() = new_input;

    return false; // true if the user wants to quit the game
}