
#pragma once

#include "resource_manager.h"

class dir_watcher;
struct spritesheet;

class allegro_resource_manager : public resource_manager {
public:
	allegro_resource_manager(std::string data_path);
	~allegro_resource_manager();

	void begin_frame() override;

	void reload_all_sprite_bitmaps() override;

	bitmap_id load_bitmap_resource(entt::hashed_string path_relative_to_data) override;
	ALLEGRO_BITMAP* get_bitmap_by_id(bitmap_id id) override;

	bitmap_id load_anim_resource(entt::hashed_string path_relative_to_data) override;
	spritesheet* get_anim_by_id(bitmap_id id) override;

	ALLEGRO_FONT* load_font(const char* path) override;

	std::optional<std::string> lookup_resource_path_by_hash(uint32_t hash) override; // editor only

private:
	std::string _data_path;
	std::unique_ptr<dir_watcher> _dir_watcher;
	std::unordered_map<uint32_t, std::string> _resource_paths_reverse_map; // editor only

	std::unordered_map<uint32_t, uint16_t> _bitmap_resource_hash_runtime_idx_map;
	std::vector<ALLEGRO_BITMAP*> _bitmaps;

	std::unordered_map<uint32_t, uint16_t> _spritesheet_resource_hash_runtime_idx_map;
	std::vector<spritesheet*> _spritesheets;

	void discover_all_bitmap_resources();
};