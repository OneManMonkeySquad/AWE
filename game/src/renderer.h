#pragma once

#include "math.h"

class dir_watcher;

struct camera {
	math::vector2 position = { 0, 0 };
	float zoom = 1.0;
	float angle = 0;
};

using bitmap_id = uint16_t;

struct sprite_instance {
	bitmap_id sprite;
};

class renderer {
public:
	renderer(std::string window_title, std::string data_path);
	~renderer();

	ALLEGRO_DISPLAY* get_display() const;

	bitmap_id load_sprite(std::string_view path_relative_to_data);
	void reload_all_sprite_bitmaps();

	void begin_frame();
	void render(const camera cam, const entt::registry& game_state);

private:
	std::string _data_path;

	ALLEGRO_DISPLAY* _display = nullptr;

	ALLEGRO_FONT* _ttf_font = nullptr;

	std::unique_ptr<dir_watcher> _dir_watcher;

	std::unordered_map<std::string, uint16_t> _path_sprite_idx_map;
	std::vector<ALLEGRO_BITMAP*> _sprites;

	bitmap_id _tile_sprite;
	int _tile_map[100 * 100];

	ALLEGRO_BITMAP* get_bitmap_by_id(bitmap_id id) {
		return _sprites[id];
	}

	void render_debug_utils_in_world_space(const entt::registry& game_state);
	void render_debug_utils_in_screen_space(const entt::registry& game_state);
};

entt::registry clone_for_rendering(const entt::registry& current_state);
entt::registry interpolate_for_rendering(const entt::registry& current, const entt::registry& previous, float a);