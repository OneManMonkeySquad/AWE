
#pragma once

#include "fabrik_math.h"

namespace component {
	struct transform {
		math::vector2 position;
		float angle;

		template<typename Archive>
		void serialize(Archive& archive) {
			archive(position, angle);
		}
	};
}