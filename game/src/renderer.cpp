
#include "pch.h"
#include "renderer.h"
#include "math.h"
#include "engine.h"
#include "utils.h"
#include "defer.h"
#include "bitmap_reader.h"
#include "dir_watcher.h"
#include "game.h"
#include "imgui\backends\imgui_impl_allegro5.h"
#if TRACY_ENABLE
#include "tracy/Tracy.hpp"
#endif

renderer::renderer(std::string window_title, std::string data_path) {
	_data_path = data_path;
	if (!_data_path.ends_with("/") && !_data_path.ends_with("\\")) {
		_data_path += "/";
	}

	if (!al_init_primitives_addon() ||
		!al_init_image_addon() ||
		!al_init_ttf_addon())
		panic("Allegro addon init failed");

	al_set_new_display_flags(ALLEGRO_WINDOWED); // ALLEGRO_FRAMELESS
	_display = al_create_display(1280, 768);
	al_set_window_title(_display, window_title.c_str());


	_ttf_font = al_load_ttf_font((_data_path + "fonts/OpenSans-Regular.ttf").c_str(), 18, 0);

	//
	_dir_watcher = std::make_unique<dir_watcher>(std::filesystem::absolute(_data_path));

	//
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();


	auto& io = ImGui::GetIO();
	io.FontDefault = io.Fonts->AddFontFromFileTTF((_data_path + "fonts/OpenSans-Regular.ttf").c_str(), 18);
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsDark();
	ImGui_ImplAllegro5_Init(_display);


	{
		_tile_sprite = load_sprite("tile.psd");

		for (int y = 0; y < 100; y++) {
			for (int x = 0; x < 100; x++) {
				_tile_map[x + y * 100] = rand() % 4;
			}
		}
	}
}

renderer::~renderer() {
	ImGui_ImplAllegro5_Shutdown();
	ImGui::DestroyContext();

	al_destroy_display(_display);
}

ALLEGRO_DISPLAY* renderer::get_display() const {
	return _display;
}


bitmap_id renderer::load_sprite(std::string_view path_relative_to_data) {
	const auto path = _data_path + std::string{ path_relative_to_data };

	auto it = _path_sprite_idx_map.find(path);
	if (it != _path_sprite_idx_map.end())
		return it->second;

	print("Loading sprite '{}'", path);

	auto newBitmap = *bitmap_reader::load_bitmap(path);
	_sprites.push_back(newBitmap);

	// Attention: Assuming no more than 2^16 sprites
	const auto id = (uint16_t)(_sprites.size() - 1);
	_path_sprite_idx_map[path] = id;
	return id;
}


void renderer::reload_all_sprite_bitmaps() {
	for (const auto entry : _path_sprite_idx_map) {
		auto bitmap = bitmap_reader::load_bitmap(entry.first);
		_sprites[entry.second] = *bitmap;
	}
}

void renderer::begin_frame() {
	if (_dir_watcher->is_directory_dirty()) {
		_dir_watcher->reset_directory_dirty();
		print("Filesystem modified, reload all sprites...");
		reload_all_sprite_bitmaps();
	}

	ImGui_ImplAllegro5_NewFrame();
	ImGui::NewFrame();
}

void renderer::render(const camera cam, const entt::registry& game_state) {
	al_set_target_backbuffer(_display);
	al_clear_to_color(al_map_rgb(30, 30, 30));



	const auto displayWidth = al_get_display_width(_display);
	const auto displayHeight = al_get_display_height(_display);

	{
		ALLEGRO_TRANSFORM transform;
		al_identity_transform(&transform);
		al_translate_transform(&transform, -cam.position.x, -cam.position.y);
		al_rotate_transform(&transform, cam.angle);
		al_scale_transform(&transform, cam.zoom, cam.zoom);
		al_translate_transform(&transform, displayWidth * 0.5, displayHeight * 0.5);
		al_use_transform(&transform);
	}

	{
		al_hold_bitmap_drawing(1);
		defer{ al_hold_bitmap_drawing(0); };

		int x, y;
		for (y = 0; y < 100; y++) {
			for (x = 0; x < 100; x++) {
				int i = _tile_map[x + y * 100];
				float u = 1;
				float v = 1;

				auto bitmap = get_bitmap_by_id(_tile_sprite);
				al_draw_scaled_bitmap(bitmap,
					u, v, 64, 32,
					x * 15.f, y * 15.f,
					64, 32,
					0);
			}
		}
	}

	{
		auto rendables = game_state.view<const transform, const sprite_instance>();

		auto depth_rendable_pairs = std::vector<std::pair<float, entt::entity>>{ rendables.size_hint() };

		for (auto e : rendables) {
			auto& tr = rendables.get<const transform>(e);
			auto& sp = rendables.get<const sprite_instance>(e);

			depth_rendable_pairs.emplace_back(tr.position.y, e);
		}

		std::sort(depth_rendable_pairs.begin(), depth_rendable_pairs.end(), [](auto& lp, auto& rp) {
			return lp.first < rp.first;
		});

		{
			al_hold_bitmap_drawing(true);
			defer{ al_hold_bitmap_drawing(false); };

			for (auto p : depth_rendable_pairs) {
				const auto& tr = rendables.get<const transform>(p.second);
				const auto sprite_inst = rendables.get<const sprite_instance>(p.second);
				const auto bitmap = get_bitmap_by_id(sprite_inst.sprite);

				const auto w = al_get_bitmap_width(bitmap);
				const auto h = al_get_bitmap_height(bitmap);

				al_draw_rotated_bitmap(bitmap, w * 0.5f, h * 0.5f, tr.position.x, tr.position.y, tr.angle * math::degrees_to_rad, 0);
			}
		}
	}

	render_debug_utils_in_world_space(game_state);

	{
		ALLEGRO_TRANSFORM transform;
		al_identity_transform(&transform);
		al_use_transform(&transform);
	}

	render_debug_utils_in_screen_space(game_state);



	// Imgui
	//ImGui::ShowDemoWindow();

	ImGui::Render();
	ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());

	al_flip_display();

#if TRACY_ENABLE
	FrameMark;
#endif
}

void renderer::render_debug_utils_in_world_space(const entt::registry& game_state) {
	auto& global_data = game_state.ctx<const debug::internal::util_global_data>();

	for (auto& line : global_data.world_lines) {
		auto from = line.from;
		auto to = line.to;
		const auto color = al_map_rgba(line.color.r, line.color.g, line.color.b, line.color.a);
		al_draw_line(from.x, from.y, to.x, to.y, color, 2);
	}

	for (auto& text : global_data.world_text) {
		const auto color = al_map_rgba(text.color.r, text.color.g, text.color.b, text.color.a);
		al_draw_text(_ttf_font, color, text.position.x, text.position.y, ALLEGRO_ALIGN_CENTRE, text.text.c_str());
	}
}

static std::chrono::time_point<std::chrono::high_resolution_clock> old_time;
static float oldFps;
void renderer::render_debug_utils_in_screen_space(const entt::registry& game_state) {
	auto& global_data = game_state.ctx<const debug::internal::util_global_data>();

	const auto displayWidth = al_get_display_width(_display);
	const auto displayHeight = al_get_display_height(_display);

	for (auto& text : global_data.screen_text) {
		const auto color = al_map_rgba(text.color.r, text.color.g, text.color.b, text.color.a);
		al_draw_text(_ttf_font, color, text.position.x, text.position.y, 0, text.text.c_str());
	}

	// FPS
	using clock = std::chrono::high_resolution_clock;

	using ms = std::chrono::duration<float, std::milli>;

	auto new_time = clock::now();
	auto delta = std::chrono::duration_cast<ms>(new_time - old_time).count();
	old_time = new_time;
	auto fps = (1 / delta) * 1000;
	fps = math::lerp(oldFps, fps, 0.02f);
	oldFps = fps;

	al_draw_text(_ttf_font, al_map_rgb(255, 255, 0), 16, 16, 0, std::format("FPS: {}", (int)fps).c_str());
}

entt::registry clone_for_rendering(const entt::registry& current_state) {
	entt::registry cloned;
	cloned.assign(current_state.data(), current_state.data() + current_state.size(), current_state.released());

	{
		auto view = current_state.view<const transform>();
		cloned.insert(view.data(), view.data() + view.size(), view.raw());
	}
	{
		auto view = current_state.view<const sprite_instance>();
		cloned.insert(view.data(), view.data() + view.size(), view.raw());
	}
	return cloned;
}


entt::registry interpolate_for_rendering(const entt::registry& current_state, const entt::registry& previous_state, float a) {
	entt::registry interpolated_state;

	interpolated_state.set<debug::internal::util_global_data>() = current_state.ctx<const debug::internal::util_global_data>();

	for (auto [e, current_transform, sp] : current_state.view<const transform, const sprite_instance>().each()) {
		math::vector2 old_pos = current_transform.position;
		float old_angle = current_transform.angle;
		if (previous_state.valid(e) && previous_state.all_of<transform>(e)) { // Entity k�nnte erst diesen Frame existieren
			const auto& old_transform = previous_state.get<const transform>(e);
			old_pos = old_transform.position;
			old_angle = old_transform.angle;
		}

		auto interp_pos = math::lerp(old_pos, current_transform.position, a);
		auto interp_angle = math::lerp(old_angle, current_transform.angle, a);

		auto interp_e = interpolated_state.create();
		interpolated_state.emplace<transform>(interp_e, interp_pos, interp_angle);
		interpolated_state.emplace<sprite_instance>(interp_e, sp);
	}

	return interpolated_state;
}
