
#pragma once

#include "scene.h"

class scene_manager {
public:
	~scene_manager();

	scene& create_scene();
	scene& get_first_scene();

private:
	std::list<std::unique_ptr<scene>> _scenes;
};