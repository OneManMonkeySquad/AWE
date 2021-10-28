
#pragma once

#include "renderer.h"

class null_renderer : public renderer {
public:
	void initialize(engine* engine) override;

	void begin_frame() override;
	void render(const component::camera cam, const entt::registry& game_state) override;

	entt::registry clone_for_rendering(const scene& scene) override;
	entt::registry interpolate_for_rendering(const entt::registry& current, const entt::registry& previous, float a) override;

private:
	std::unordered_map<std::string, uint16_t> _path_sprite_idx_map;
};