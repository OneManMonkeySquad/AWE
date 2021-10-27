#include "pch.h"
#include "resource_manager.h"
#include "dir_watcher.h"
#include "bitmap_reader.h"

// http://bitsquid.blogspot.com/2011/12/platform-specific-resources.html

resource_manager::resource_manager(std::string data_path)
	: _data_path{ data_path } {
	discover_all_bitmap_resources();

	_dir_watcher = std::make_unique<dir_watcher>(std::filesystem::absolute(data_path));
}

resource_manager::~resource_manager() {}

void resource_manager::begin_frame() {
	if (_dir_watcher->is_directory_dirty()) {
		_dir_watcher->reset_directory_dirty();
		discover_all_bitmap_resources();
		reload_all_sprite_bitmaps();
	}
}

void resource_manager::reload_all_sprite_bitmaps() {
	for (const auto& entry : _bitmap_resource_hash_runtime_idx_map) {
		const auto& path = _bitmap_reverse_map[entry.first];
		auto bitmap = bitmap_reader::load_bitmap(_data_path + path);
		_bitmaps[entry.second] = *bitmap;
	}
}

bitmap_id resource_manager::load_bitmap_resource(entt::hashed_string path_relative_to_data) {
	const auto it = _bitmap_resource_hash_runtime_idx_map.find(path_relative_to_data);
	if (it != _bitmap_resource_hash_runtime_idx_map.end())
		return { it->second, path_relative_to_data.value() };

	const auto path = _data_path + path_relative_to_data.data();
	print("Loading sprite {}", path);

	auto new_bitmap = *bitmap_reader::load_bitmap(path);
	_bitmaps.push_back(new_bitmap);

	// Attention: Assuming no more than 2^16 sprites
	const auto id = (uint16_t)(_bitmaps.size() - 1);
	_bitmap_resource_hash_runtime_idx_map[path_relative_to_data] = id;
	return { id, path_relative_to_data.value() };
}

ALLEGRO_BITMAP* resource_manager::get_bitmap_by_id(bitmap_id id) {
	return _bitmaps[id.runtime_idx];
}

ALLEGRO_FONT* resource_manager::load_font(const char* path) {
	return al_load_ttf_font((_data_path + path).c_str(), 18, 0);
}

std::optional<std::string> resource_manager::lookup_resource_path_by_hash(uint32_t hash) {
	auto it = _bitmap_reverse_map.find(hash);
	if (it == _bitmap_reverse_map.end())
		return {};

	return it->second;
}

void resource_manager::discover_all_bitmap_resources() {
	_bitmap_reverse_map.clear();

	for (auto const& dir_entry : std::filesystem::recursive_directory_iterator{ _data_path }) {
		if (!dir_entry.is_regular_file())
			continue;

		const auto path = dir_entry.path().generic_string().substr(_data_path.size());
		_bitmap_reverse_map[entt::hashed_string::value(path.data(), path.size())] = path;
	}
}