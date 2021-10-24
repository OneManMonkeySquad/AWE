#pragma once

#include "game.h"


struct ai_agent;


enum class action_result {
	in_progress,
	failed,
	succeeded
};


struct action_context {
	ai_agent& agent;
	entt::entity agent_entity;
};

action_result hunt_deer(scene& scene, action_context& ctx);
action_result pickup_meat(scene& scene, action_context& ctx);
action_result eat_meat(scene& scene, action_context& ctx);
