#include "pch.h"
#include "imgui\backends\imgui_impl_allegro5.h"
#include "input.h"
#include "engine.h"
#include "utils.h"

bool input::is_key_down(int key) {
	return key_down[key];
}

int input::get_mouse_wheel() {
	return mouseWheel;
}

void input::begin_frame() {
	mouseWheel = 0;
	events.clear();
}

void input::on_event(ALLEGRO_EVENT ev) {
	ImGui_ImplAllegro5_ProcessEvent(&ev);

	ImGuiIO& io = ImGui::GetIO();

	switch (ev.type) {
	case ALLEGRO_EVENT_DISPLAY_RESIZE:
		panic("Resize Not Implemented");
		break;

	case ALLEGRO_EVENT_KEY_DOWN:
		if (!io.WantCaptureKeyboard) {
			key_down[ev.keyboard.keycode] = true;
			events.emplace_back(input_event_type::key_down, ev.keyboard.keycode);
		}
		break;

	case ALLEGRO_EVENT_KEY_UP:
		// io.WantCaptureKeyboard absichtlich ignorieren
		key_down[ev.keyboard.keycode] = false;
		events.emplace_back(input_event_type::key_up, ev.keyboard.keycode);
		break;

	case ALLEGRO_EVENT_MOUSE_AXES:
		if (!io.WantCaptureMouse) {
			mouseWheel = ev.mouse.dz;
		}
		break;

	case ALLEGRO_EVENT_DISPLAY_SWITCH_OUT:
		for (int i = 0; i < ALLEGRO_KEY_MAX; ++i) {
			key_down[i] = false;
		}
		break;
	}
}