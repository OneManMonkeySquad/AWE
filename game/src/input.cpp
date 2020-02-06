
#include "pch.h"
#include "input.h"
#include "engine.h"
#include "imgui-1.74\imgui.h"
#include "imgui-1.74\examples\imgui_impl_allegro5.h"

static bool key_down[ALLEGRO_KEY_MAX] = {};

bool handle_input(engine_state& engine, entt::registry& state) {
    input new_input;
    new_input.wheel = 0;

    ImGuiIO& io = ImGui::GetIO();

    ALLEGRO_EVENT ev;
    while (al_get_next_event(engine.event_queue, &ev)) {
        ImGui_ImplAllegro5_ProcessEvent(&ev);

        

        switch (ev.type) {
        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            return true; // quit game

        case ALLEGRO_EVENT_KEY_DOWN:
            if (!io.WantCaptureKeyboard) {
                key_down[ev.keyboard.keycode] = true;
            }
            break;

        case ALLEGRO_EVENT_KEY_UP:
            // io.WantCaptureKeyboard absichtlich ignorieren
            key_down[ev.keyboard.keycode] = false;
            break;

        case ALLEGRO_EVENT_MOUSE_AXES:
            if (!io.WantCaptureMouse) {
                new_input.wheel = ev.mouse.dz;
            }
            break;
        }
    }

    if (key_down[ALLEGRO_KEY_ESCAPE])
        return true; // Spiel beenden

    new_input.w = key_down[ALLEGRO_KEY_W];
    new_input.s = key_down[ALLEGRO_KEY_S];
    new_input.a = key_down[ALLEGRO_KEY_A];
    new_input.d = key_down[ALLEGRO_KEY_D];

    state.set<input>() = new_input;

    return false; // true if the user wants to quit the game
}