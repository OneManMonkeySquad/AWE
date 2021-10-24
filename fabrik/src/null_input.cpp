
#include "pch.h"
#include "null_input.h"

void null_input::initialize() {}

void null_input::begin_frame() {}

bool null_input::is_key_down(int key) {
	return false;
}

int null_input::get_mouse_wheel() {
	return 0;
}
