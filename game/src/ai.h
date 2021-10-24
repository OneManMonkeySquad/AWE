#pragma once

#include "ai_actions.h"

struct ai_agent {
	entt::entity target;
	float hunger;

	template<typename Archive>
	void serialize(Archive& archive) {
		archive(target, hunger);
	}
};

void init_ai(scene& scene);
void update_ai(scene& scene);
