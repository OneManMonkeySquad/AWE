
#pragma once

#include "fabrik_math.h"
#include "resource_manager.h"

class scene;
class engine;

namespace component {
	struct camera {
		math::vector2 position = { 0, 0 };
		float zoom = 1.0;
		float angle = 0;
	};

	struct sprite_instance {
		bitmap_id bitmap;

		template<typename Archive>
		void serialize(Archive& archive) {
			archive(bitmap);
		}
	};
}

class renderer {
public:
	virtual ~renderer() = default;

	virtual void initialize(engine* engine) = 0;

	virtual void begin_frame() = 0;
	virtual void render(const component::camera cam, const entt::registry& registry) = 0;

	virtual entt::registry clone_for_rendering(const scene& scene) = 0;
	virtual entt::registry interpolate_for_rendering(const entt::registry& current_registry, const entt::registry& previous_registry, float a) = 0;
};