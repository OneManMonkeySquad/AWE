
#pragma once

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
	virtual ~resource_manager() = default;

	virtual void begin_frame() = 0;

	virtual void reload_all_sprite_bitmaps() = 0;

	virtual bitmap_id load_bitmap_resource(entt::hashed_string path_relative_to_data) = 0;
	virtual ALLEGRO_BITMAP* get_bitmap_by_id(bitmap_id id) = 0;

	virtual ALLEGRO_FONT* load_font(const char* path) = 0;

	virtual std::optional<std::string> lookup_resource_path_by_hash(uint32_t hash) = 0; // editor only
};