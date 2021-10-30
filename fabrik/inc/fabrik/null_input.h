
#pragma once

#include "input.h"

class null_input : public input {
public:
	void initialize() override;

	void begin_frame() override;

	input_method get_current_input_method() const override;

	bool is_key_down(int key) const override;

	int get_mouse_wheel() const override;
	bool is_mouse_button_down(int button) const override;
	math::vector2 get_mouse_position() const override;

	float get_joystick_axis(int stick, int axis) const override;
	bool get_joystick_button(int button) const override;
};