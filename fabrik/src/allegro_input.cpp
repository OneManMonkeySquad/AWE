#include "pch.h"
#include "imgui\backends\imgui_impl_allegro5.h"
#include "allegro_input.h"
#include "engine.h"
#include "utils.h"

allegro_input::~allegro_input() {
	al_destroy_event_queue(_event_queue);
}

void allegro_input::initialize() {
	_event_queue = al_create_event_queue();
	al_register_event_source(_event_queue, al_get_keyboard_event_source());
	al_register_event_source(_event_queue, al_get_mouse_event_source());
}

bool allegro_input::is_key_down(int key) {
	return key_down[key];
}

int allegro_input::get_mouse_wheel() {
	return mouseWheel;
}

void allegro_input::begin_frame() {
	mouseWheel = 0;
	events.clear();

	ALLEGRO_EVENT ev;
	while (al_get_next_event(_event_queue, &ev)) {
		on_event(ev);

		//if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
		//	return true;
	}
}

void allegro_input::on_event(ALLEGRO_EVENT ev) {
	ImGui_ImplAllegro5_ProcessEvent(&ev);

	ImGuiIO& io = ImGui::GetIO();

	switch (ev.type) {
	case ALLEGRO_EVENT_DISPLAY_RESIZE:
		// #todo
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