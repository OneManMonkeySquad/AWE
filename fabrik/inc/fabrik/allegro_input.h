#pragma once

#include "input.h"
#include "fabrik_math.h"

class allegro_input : public input {
public:
	~allegro_input();

	void initialize() override;

	void begin_frame() override;

	input_method get_current_input_method() const override;

	bool is_key_down(int key) const override;

	int get_mouse_wheel() const override;
	bool is_mouse_button_down(int button) const override;
	math::vector2 get_mouse_position() const override;

	float get_joystick_axis(int stick, int axis) const override;
	bool get_joystick_button(int button) const override;

private:
	ALLEGRO_EVENT_QUEUE* _event_queue = nullptr;

	bool _key_down[ALLEGRO_KEY_MAX] = {};
	math::vector2 _mouse_position{};
	int _mouseWheel = 0;
	bool _mouse_button[6];
	float _joystick_axis[3][2];
	bool _joystick_buttons[12];
	input_method _current_input_method = input_method::keyboard_mouse;

	void on_event(ALLEGRO_EVENT ev);
};
