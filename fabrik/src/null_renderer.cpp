
#include "pch.h"
#include "null_renderer.h"

void null_renderer::initialize(engine* engine) {}

void null_renderer::begin_frame() {}

void null_renderer::render(const camera cam, const entt::registry& game_state) {}

entt::registry null_renderer::clone_for_rendering(const scene& scene) {
	return {};
}

entt::registry null_renderer::interpolate_for_rendering(const entt::registry& current, const entt::registry& previous, float a) {
	return {};
}
