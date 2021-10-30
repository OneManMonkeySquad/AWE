
#include "pch.h"
#include "null_input.h"
#include "fabrik_math.h"

void null_input::initialize() {}

void null_input::begin_frame() {}

bool null_input::is_key_down(int key) const {
	return false;
}

int null_input::get_mouse_wheel() const {
	return 0;
}

bool null_input::is_mouse_button_down(int button) const {
	return false;
}

math::vector2 null_input::get_mouse_position() const {
	return {};
}

input_method null_input::get_current_input_method() const {
	return input_method::keyboard_mouse;
}

float null_input::get_joystick_axis(int stick, int axis) const {
	return 0;
}

bool null_input::get_joystick_button(int button) const {
	return false;
}
