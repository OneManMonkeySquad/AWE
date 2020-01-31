
#include "pch.h"
#include "engine.h"
#include "game.h"
#include "renderer.h"
#include "utils.h"

using namespace std;

struct input {
    bool w, a, s, d;
    float wheel;
};

bool handle_events(engine_state& engine, input& input) {
    ALLEGRO_KEYBOARD_STATE keyboard_state;
    al_get_keyboard_state(&keyboard_state);

    input.w = al_key_down(&keyboard_state, ALLEGRO_KEY_W);
    input.s = al_key_down(&keyboard_state, ALLEGRO_KEY_S);
    input.a = al_key_down(&keyboard_state, ALLEGRO_KEY_A);
    input.d = al_key_down(&keyboard_state, ALLEGRO_KEY_D);

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
            input.wheel = ev.mouse.dz;
            break;
        }
    }

    return false; // true if the user wants to quit the game
}

int main(int argc, char** argv) {
    using clock = chrono::high_resolution_clock;

    constexpr chrono::nanoseconds timestep = 16ms;

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

    auto engine = create_engine();

    auto renderer = create_renderer(*engine);

    chrono::nanoseconds lag = 0ns;
    auto time_start = clock::now();

    entt::registry current_state;
    init_game(current_state);

    entt::registry previous_state = current_state.clone(); // Sicher stellen, dass previous_state im ersten Frame gültig ist

    bool quit_game = false;
    while (!quit_game) {
        const auto delta_time = clock::now() - time_start;

        using ms = chrono::duration<float, std::milli>;
        const auto delta_time_ms = chrono::duration_cast<ms>(delta_time).count();

        time_start = clock::now();
        lag += chrono::duration_cast<chrono::nanoseconds>(delta_time);

        input new_input{};
        quit_game = handle_events(*engine, new_input);

        // update game logic as lag permits
        while (lag >= timestep) {
            lag -= timestep;

            previous_state = current_state.clone();
            update(current_state); // update at a fixed rate each time
        }

        // calculate how close or far we are from the next timestep
        auto a = (float)lag.count() / timestep.count();
        auto interpolated_state = interpolate_for_rendering(current_state, previous_state, a);

        renderer->main_camera.zoom = clamp(renderer->main_camera.zoom + new_input.wheel * delta_time_ms * 0.05f, 0.4f, 1.8f);
        renderer->main_camera.position.x += (new_input.d - new_input.a) * delta_time_ms * renderer->main_camera.zoom * 0.5f;
        renderer->main_camera.position.y += (new_input.s - new_input.w) * delta_time_ms * renderer->main_camera.zoom * 0.5f;

        render(*renderer, *engine, interpolated_state);
    }

    destroy_engine(engine);

    return 0;
}