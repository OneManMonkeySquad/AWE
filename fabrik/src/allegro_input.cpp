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
	al_register_event_source(_event_queue, al_get_joystick_event_source());
}

bool allegro_input::is_key_down(int key) const {
	return _key_down[key];
}

int allegro_input::get_mouse_wheel() const {
	return _mouseWheel;
}

bool allegro_input::is_mouse_button_down(int button) const {
	return _mouse_button[button];
}

math::vector2 allegro_input::get_mouse_position() const {
	return _mouse_position;
}

float allegro_input::get_joystick_axis(int stick, int axis) const {
	return _joystick_axis[stick][axis];
}

bool allegro_input::get_joystick_button(int button) const {
	return _joystick_buttons[button];
}

void allegro_input::begin_frame() {
	_mouseWheel = 0;
	events.clear();

	ALLEGRO_EVENT ev;
	while (al_get_next_event(_event_queue, &ev)) {
		on_event(ev);

		//if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
		//	return true;
	}
}

input_method allegro_input::get_current_input_method() const {
	return _current_input_method;
}

void allegro_input::on_event(ALLEGRO_EVENT ev) {
	ImGui_ImplAllegro5_ProcessEvent(&ev);

	ImGuiIO& io = ImGui::GetIO();

	switch (ev.type) {
	case ALLEGRO_EVENT_DISPLAY_RESIZE:
		// #todo
		break;

	case ALLEGRO_EVENT_KEY_DOWN:
		_current_input_method = input_method::keyboard_mouse;
		if (!io.WantCaptureKeyboard) {
			_key_down[ev.keyboard.keycode] = true;
			events.emplace_back(input_event_type::key_down, ev.keyboard.keycode);
		}
		break;

	case ALLEGRO_EVENT_KEY_UP:
		// io.WantCaptureKeyboard absichtlich ignorieren
		_key_down[ev.keyboard.keycode] = false;
		events.emplace_back(input_event_type::key_up, ev.keyboard.keycode);
		break;

	case ALLEGRO_EVENT_MOUSE_AXES:
		if (!io.WantCaptureMouse) {
			_mouse_position.x = ev.mouse.x;
			_mouse_position.y = ev.mouse.y;
			_mouseWheel = ev.mouse.dz;
		}
		break;

	case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
		_mouse_button[ev.mouse.button - 1] = true;
		break;

	case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
		_mouse_button[ev.mouse.button - 1] = false;
		break;

	case ALLEGRO_EVENT_JOYSTICK_AXIS:
		if (ev.joystick.pos > 0.25f) {
			_current_input_method = input_method::joystick;
		}
		_joystick_axis[ev.joystick.stick][ev.joystick.axis] = ev.joystick.pos;
		break;

	case ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN:
		_current_input_method = input_method::joystick;
		_joystick_buttons[ev.joystick.button] = true;
		break;

	case ALLEGRO_EVENT_JOYSTICK_BUTTON_UP:
		_joystick_buttons[ev.joystick.button] = false;
		break;

	case ALLEGRO_EVENT_DISPLAY_SWITCH_OUT:
		memset(_key_down, false, std::size(_key_down));
		memset(_joystick_buttons, false, std::size(_joystick_buttons));
		break;
	}
}