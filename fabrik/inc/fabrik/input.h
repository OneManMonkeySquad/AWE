
#pragma once

enum class input_event_type {
	key_down,
	key_up
};

struct input_event {
	input_event_type type;
	int key;
};

enum input_method {
	keyboard_mouse,
	joystick
};

const int input_xbox_left_stick = 0;
const int input_xbox_right_stick = 1;

class input {
public:
	std::vector<input_event> events;

	virtual ~input() = default;

	virtual void initialize() = 0;

	virtual void begin_frame() = 0;

	virtual input_method get_current_input_method() const = 0;
	virtual bool is_key_down(int key) const = 0;
	virtual int get_mouse_wheel() const = 0;
	virtual float get_joystick_axis(int stick, int axis) const = 0;
};