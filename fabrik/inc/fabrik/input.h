
#pragma once

enum class input_event_type {
	key_down,
	key_up
};

struct input_event {
	input_event_type type;
	int key;
};

class input {
public:
	std::vector<input_event> events;

	virtual ~input() = default;

	virtual void initialize() = 0;

	virtual void begin_frame() = 0;

	virtual bool is_key_down(int key) = 0;
	virtual int get_mouse_wheel() = 0;
};