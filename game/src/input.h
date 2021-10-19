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

	bool is_key_down(int key);

	int get_mouse_wheel();

	void begin_frame();
	void on_event(ALLEGRO_EVENT ev);

private:
	bool key_down[ALLEGRO_KEY_MAX] = {};
	int mouseWheel = 0;
};
