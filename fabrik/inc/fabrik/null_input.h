
#pragma once

#include "input.h"

class null_input : public input {
public:
	void initialize() override;

	void begin_frame() override;

	bool is_key_down(int key) override;
	int get_mouse_wheel() override;
};