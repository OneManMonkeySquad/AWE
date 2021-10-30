#include "pch.h"
#include "allegro_resource_manager.h"
#include "dir_watcher.h"
#include "bitmap_reader.h"
#include "jobs.h"
#include "renderer.h"

// http://bitsquid.blogspot.com/2011/12/platform-specific-resources.html

allegro_resource_manager::allegro_resource_manager(std::string data_path)
	: _data_path{ data_path } {
	discover_all_bitmap_resources();

	_dir_watcher = std::make_unique<dir_watcher>(std::filesystem::absolute(data_path));
}

allegro_resource_manager::~allegro_resource_manager() {}

void allegro_resource_manager::begin_frame() {
	if (_dir_watcher->is_directory_dirty()) {
		_dir_watcher->reset_directory_dirty();
		discover_all_bitmap_resources();
		reload_all_sprite_bitmaps();
	}
}

void allegro_resource_manager::reload_all_sprite_bitmaps() {
	for (const auto& entry : _bitmap_resource_hash_runtime_idx_map) {
		const auto& path = _resource_paths_reverse_map[entry.first];
		auto bitmap = bitmap_reader::load_bitmap(_data_path + path);
		_bitmaps[entry.second] = *bitmap;
	}
}

bitmap_id allegro_resource_manager::load_bitmap_resource(entt::hashed_string path_relative_to_data) {
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

ALLEGRO_BITMAP* allegro_resource_manager::get_bitmap_by_id(bitmap_id id) {
	return _bitmaps[id.runtime_idx];
}

bitmap_id allegro_resource_manager::load_anim_resource(entt::hashed_string path_relative_to_data) {
	const auto it = _spritesheet_resource_hash_runtime_idx_map.find(path_relative_to_data);
	if (it != _spritesheet_resource_hash_runtime_idx_map.end())
		return { it->second, path_relative_to_data.value() };

	const auto path = _data_path + path_relative_to_data.data();
	print("Loading anim {}", path);

	spritesheet* new_spritesheet;
	{
		auto file_content = jobs::read_file(path);
		auto lines = split_string(file_content, '\n');

		new_spritesheet = new spritesheet;

		for (const auto& line : lines) {
			// Spellcast_U,0,0,448,64

			const auto tokens = split_string(line, ',');
			fabrik_assert(tokens.size() == 5);

			new_spritesheet->entry_names.emplace_back(tokens[0]);

			spritesheet_entry new_entry;

			auto base_x = *parse_int(tokens[1]);
			auto base_y = *parse_int(tokens[2]);
			auto height = *parse_int(tokens[4]);
			auto num = *parse_int(tokens[3]) / height;
			for (int i = 0; i < num; ++i) {
				sprite_frame frame;
				frame.position.x = base_x + i * height;
				frame.position.y = base_y;
				frame.size.x = height;
				frame.size.y = height;

				new_entry.push_back(std::move(frame));
			}

			new_spritesheet->animations.push_back(std::move(new_entry));
		}
	}

	_spritesheets.push_back(new_spritesheet);

	// Attention: Assuming no more than 2^16 sprites
	const auto id = (uint16_t)(_spritesheets.size() - 1);
	_spritesheet_resource_hash_runtime_idx_map[path_relative_to_data] = id;
	return { id, path_relative_to_data.value() };
}

spritesheet* allegro_resource_manager::get_anim_by_id(bitmap_id id) {
	return _spritesheets[id.runtime_idx];
}

ALLEGRO_FONT* allegro_resource_manager::load_font(const char* path) {
	return al_load_ttf_font((_data_path + path).c_str(), 18, 0);
}

std::optional<std::string> allegro_resource_manager::lookup_resource_path_by_hash(uint32_t hash) {
	auto it = _resource_paths_reverse_map.find(hash);
	if (it == _resource_paths_reverse_map.end())
		return {};

	return it->second;
}

void allegro_resource_manager::discover_all_bitmap_resources() {
	_resource_paths_reverse_map.clear();

	for (auto const& dir_entry : std::filesystem::recursive_directory_iterator{ _data_path }) {
		if (!dir_entry.is_regular_file())
			continue;

		const auto path = dir_entry.path().generic_string().substr(_data_path.size());
		_resource_paths_reverse_map[entt::hashed_string::value(path.data(), path.size())] = path;
	}
}