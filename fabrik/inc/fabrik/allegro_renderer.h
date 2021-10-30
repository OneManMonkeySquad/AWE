#pragma once

#include "renderer.h"

class scene;

class allegro_renderer : public renderer {
public:
	allegro_renderer(std::string window_title);
	~allegro_renderer();

	void initialize(engine* engine) override;

	void begin_frame() override;
	void tick() override;

	void render(const component::camera cam, const entt::registry& registry) override;

	entt::registry clone_for_rendering(const scene& scene) override;
	entt::registry interpolate_for_rendering(const entt::registry& current_registry,
											 const entt::registry& previous_registry,
											 float a) override;

private:
	std::string _window_title;
	engine* _engine;

	ALLEGRO_DISPLAY* _display = nullptr;

	ALLEGRO_FONT* _ttf_font = nullptr;

	bitmap_id _tile_sprite;
	int _tile_map[100 * 100];

	void render_debug_utils_in_world_space(const entt::registry& registry);
	void render_debug_utils_in_screen_space(const entt::registry& registry);
};