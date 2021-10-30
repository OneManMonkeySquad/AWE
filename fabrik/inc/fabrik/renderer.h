
#pragma once

#include "fabrik_math.h"
#include "resource_manager.h"

class scene;
class engine;

struct sprite_frame {
	math::vector2 position;
	math::vector2 size;
};
using spritesheet_entry = std::vector<sprite_frame>;

struct spritesheet {
	std::vector<std::string> entry_names;
	std::vector<spritesheet_entry> animations;
};

namespace component {
	struct camera {
		math::vector2 position;
		float zoom = 1;
		float angle = 0;
	};

	struct sprite_instance {
		bitmap_id bitmap;

		template<typename Archive>
		void serialize(Archive& archive) {
			archive(bitmap);
		}
	};

	struct animated_sprite {
		bitmap_id spritesheet;
		uint8_t animation_idx = 0;
		uint8_t current_frame = 0;
		uint16_t ticks_till_next_frame = 0;

		template<typename Archive>
		void serialize(Archive& archive) {
			archive(spritesheet, animation_idx, current_frame, ticks_till_next_frame);
		}
	};
}

class renderer {
public:
	virtual ~renderer() = default;

	virtual void initialize(engine* engine) = 0;

	virtual void begin_frame() = 0;
	virtual void tick() = 0;

	virtual void render(const component::camera cam, const entt::registry& registry) = 0;

	virtual entt::registry clone_for_rendering(const scene& scene) = 0;
	virtual entt::registry interpolate_for_rendering(const entt::registry& current_registry, const entt::registry& previous_registry, float a) = 0;
};