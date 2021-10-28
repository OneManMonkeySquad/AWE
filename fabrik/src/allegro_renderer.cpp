
#include "pch.h"
#include "allegro_renderer.h"
#include "math.h"
#include "engine.h"
#include "utils.h"
#include "defer.h"
#include "bitmap_reader.h"
#include "dir_watcher.h"
#include "transform.h"
#include "imgui\backends\imgui_impl_allegro5.h"
#include "scene.h"

allegro_renderer::allegro_renderer(std::string window_title)
	: _window_title(window_title) {}

allegro_renderer::~allegro_renderer() {
	ImGui_ImplAllegro5_Shutdown();
	ImGui::DestroyContext();

	al_destroy_display(_display);
}

void allegro_renderer::initialize(engine* engine) {
	_engine = engine;

	if (!al_init_primitives_addon() ||
		!al_init_image_addon() ||
		!al_init_ttf_addon())
		panic("Allegro addon init failed");

	al_set_new_display_flags(ALLEGRO_WINDOWED); // ALLEGRO_FRAMELESS
	_display = al_create_display(800, 600);
	al_set_window_title(_display, _window_title.c_str());


	_ttf_font = _engine->get_resource_manager().load_font("fonts/OpenSans-Regular.ttf");

	//
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();


	auto& io = ImGui::GetIO();
	io.FontDefault = io.Fonts->AddFontFromFileTTF("data/fonts/OpenSans-Regular.ttf", 18);
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsDark();
	ImGui_ImplAllegro5_Init(_display);


	{
		_tile_sprite = _engine->get_resource_manager().load_bitmap_resource("tile.psd");

		for (int y = 0; y < 24; y++) {
			for (int x = 0; x < 24; x++) {
				_tile_map[x + y * 24] = rand() % 4;
			}
		}
	}
}

void allegro_renderer::begin_frame() {
	ImGui_ImplAllegro5_NewFrame();
	ImGui::NewFrame();
}

void allegro_renderer::render(const component::camera cam, const entt::registry& registry) {
	ZoneScoped;

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
		ZoneScopedN("Tilemap");

		al_hold_bitmap_drawing(1);
		defer{ al_hold_bitmap_drawing(0); };

		int x, y;
		for (y = 0; y < 24; y++) {
			for (x = 0; x < 24; x++) {
				int i = _tile_map[x + y * 24];
				float u = 1;
				float v = 1;

				auto bitmap = _engine->get_resource_manager().get_bitmap_by_id(_tile_sprite);
				al_draw_bitmap(bitmap, x * 64, y * 64, 0);
			}
		}
	}

	{
		ZoneScopedN("Sprites");

		auto rendables = registry.view<const component::transform, const component::sprite_instance>();

		auto depth_rendable_pairs = std::vector<std::pair<float, entt::entity>>{ rendables.size_hint() };

		for (auto [e, tr, sp] : rendables.each()) {
			depth_rendable_pairs.emplace_back(tr.position.y, e);
		}

		std::sort(depth_rendable_pairs.begin(), depth_rendable_pairs.end(), [](auto& lp, auto& rp) {
			return lp.first < rp.first;
		});

		{
			al_hold_bitmap_drawing(true);
			defer{ al_hold_bitmap_drawing(false); };

			for (auto p : depth_rendable_pairs) {
				const auto& tr = rendables.get<const component::transform>(p.second);
				const auto sprite_inst = rendables.get<const component::sprite_instance>(p.second);
				const auto bitmap = _engine->get_resource_manager().get_bitmap_by_id(sprite_inst.bitmap);

				const auto w = al_get_bitmap_width(bitmap);
				const auto h = al_get_bitmap_height(bitmap);

				al_draw_rotated_bitmap(bitmap, w * 0.5f, h * 0.5f, tr.position.x, tr.position.y, tr.angle * math::degrees_to_rad, 0);
			}
		}
	}

	render_debug_utils_in_world_space(registry);

	{
		ALLEGRO_TRANSFORM transform;
		al_identity_transform(&transform);
		al_use_transform(&transform);
	}

	render_debug_utils_in_screen_space(registry);



	// Imgui
	//ImGui::ShowDemoWindow();

	ImGui::Render();
	ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());

	al_flip_display();

	FrameMark;
}

void allegro_renderer::render_debug_utils_in_world_space(const entt::registry& registry) {
	auto& global_data = registry.ctx<const debug::internal::util_global_data>();

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

void allegro_renderer::render_debug_utils_in_screen_space(const entt::registry& registry) {
	static std::chrono::time_point<std::chrono::high_resolution_clock> old_time;
	static float oldFps = 0;

	auto& global_data = registry.ctx<const debug::internal::util_global_data>();

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

	al_draw_text(_ttf_font, al_map_rgb(255, 255, 0), 200, 16, 0, std::format("FPS: {}", (int)fps).c_str());
}

entt::registry allegro_renderer::clone_for_rendering(const scene& scene) {
	entt::registry cloned_registry;
	cloned_registry.assign(scene.registry.data(), scene.registry.data() + scene.registry.size(), scene.registry.released());

	{
		auto view = scene.registry.view<const component::transform>();
		cloned_registry.insert(view.data(), view.data() + view.size(), view.raw());
	}
	{
		auto view = scene.registry.view<const component::sprite_instance>();
		cloned_registry.insert(view.data(), view.data() + view.size(), view.raw());
	}
	return cloned_registry;
}


entt::registry allegro_renderer::interpolate_for_rendering(const entt::registry& current_registry, const entt::registry& previous_registry, float a) {
	entt::registry interpolated_registry;

	interpolated_registry.set<debug::internal::util_global_data>() = current_registry.ctx<const debug::internal::util_global_data>();

	for (auto [e, current_transform, sp] : current_registry.view<const component::transform, const component::sprite_instance>().each()) {
		math::vector2 old_pos = current_transform.position;
		float old_angle = current_transform.angle;
		if (previous_registry.valid(e) && previous_registry.all_of<component::transform>(e)) { // Entity könnte erst diesen Frame existieren
			const auto& old_transform = previous_registry.get<const component::transform>(e);
			old_pos = old_transform.position;
			old_angle = old_transform.angle;
		}

		auto interp_pos = math::lerp(old_pos, current_transform.position, a);
		auto interp_angle = math::lerp(old_angle, current_transform.angle, a);

		auto interp_e = interpolated_registry.create();
		interpolated_registry.emplace<component::transform>(interp_e, interp_pos, interp_angle);
		interpolated_registry.emplace<component::sprite_instance>(interp_e, sp);
	}

	return interpolated_registry;
}
