#include "pch.h"
#include "scene_manager.h"

scene_manager::~scene_manager() {}

scene& scene_manager::create_scene() {
	_scenes.push_back(std::make_unique<scene>());

	return *_scenes.back().get();
}

scene& scene_manager::get_first_scene() {
	return *_scenes.front();
}