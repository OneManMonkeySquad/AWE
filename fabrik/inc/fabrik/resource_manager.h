
#pragma once



class dir_watcher;

struct bitmap_id {
	uint16_t runtime_idx;
	uint32_t resource_hash;

	template<typename Archive>
	void serialize(Archive& archive) {
		archive(runtime_idx, resource_hash);
	}
};

class resource_manager {
public:
	resource_manager(std::string data_path);
	~resource_manager();

	void begin_frame();

	void reload_all_sprite_bitmaps();

	bitmap_id load_bitmap_resource(entt::hashed_string path_relative_to_data);
	ALLEGRO_BITMAP* get_bitmap_by_id(bitmap_id id);

	ALLEGRO_FONT* load_font(const char* path);

	std::optional<std::string> lookup_resource_path_by_hash(uint32_t hash); // editor only

private:
	std::string _data_path;
	std::unique_ptr<dir_watcher> _dir_watcher;

	std::unordered_map<uint32_t, uint16_t> _bitmap_resource_hash_runtime_idx_map;
	std::vector<ALLEGRO_BITMAP*> _bitmaps;
	std::unordered_map<uint32_t, std::string> _bitmap_reverse_map; // editor only

	void discover_all_bitmap_resources();
};