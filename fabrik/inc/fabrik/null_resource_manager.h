
#pragma once

#include "resource_manager.h"

class null_resource_manager : public resource_manager {

public:
	void begin_frame() override {}

	void reload_all_sprite_bitmaps() override {}

	bitmap_id load_bitmap_resource(entt::hashed_string path_relative_to_data) override {
		return { (uint16_t)-1, path_relative_to_data.value() };
	}

	ALLEGRO_BITMAP* get_bitmap_by_id(bitmap_id id) override {
		return nullptr;
	}

	ALLEGRO_FONT* load_font(const char* path) override {
		return nullptr;
	}

	std::optional<std::string> lookup_resource_path_by_hash(uint32_t hash) override {
		return {};
	}

};