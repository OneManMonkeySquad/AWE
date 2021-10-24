#pragma once

#include "input.h"


class allegro_input : public input {
public:
	~allegro_input();

	void initialize();

	void begin_frame();

	bool is_key_down(int key);
	int get_mouse_wheel();

private:
	ALLEGRO_EVENT_QUEUE* _event_queue = nullptr;

	bool key_down[ALLEGRO_KEY_MAX] = {};
	int mouseWheel = 0;

	void on_event(ALLEGRO_EVENT ev);
};
