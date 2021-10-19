#pragma once

#include "math.h"

class engine;
class input;

struct global_state {
	int tick = 0;
	bool paused = false;
};

struct transform {
	math::vector2 position;
	float angle;
};

struct velocity {
	float dx;
	float dy;
	float angular;
};

enum class item_type {
	axe,
	meat,
	wood
};

struct item {
	item_type type;
};

struct inventory {
	std::array<entt::entity, 3> items = { entt::null, entt::null, entt::null };
};

struct tree {};
struct deer {};
struct kill {};

entt::registry game_create(engine& engine);

void game_create_commands_from_input(entt::registry& state, input& input);
void game_run_tick(entt::registry& state, engine& engine);

void kill_deer(entt::registry& state, entt::entity target);

bool inventory_add_item(entt::registry& state, entt::entity inventory_owner, entt::entity new_item);
bool inventory_has_item_of_type(entt::registry& state, entt::entity inventory_owner, item_type type);
entt::entity inventory_remove_item_of_type(entt::registry& state, entt::entity inventory_owner, item_type type);