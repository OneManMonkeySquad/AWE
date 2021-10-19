#include "pch.h"
#include "game.h"
#include "renderer.h"
#include "input.h"
#include "ai.h"
#include "utils.h"
#include "engine.h"


class command {
public:
	virtual ~command() = default;

	virtual void exec(entt::registry& state) = 0;
};

class pause_command : public command {
public:
	virtual void exec(entt::registry& state) {
		auto& ps = state.ctx<global_state>();
		ps.paused = !ps.paused;
	}
};


struct command_queue {
	std::vector<std::shared_ptr<command>> commands;
};


void game_create_commands_from_input(entt::registry& state, input& input) {
	auto& queue = state.ctx_or_set<command_queue>();

	for (auto& evt : input.events) {
		if (evt.type == input_event_type::key_down && evt.key == ALLEGRO_KEY_SPACE) {
			queue.commands.push_back(std::make_shared<pause_command>());
		}
	}
}




entt::registry game_create(engine& engine) {
	auto& renderer = engine.get_renderer();

	entt::registry state;
	state.set<global_state>();

	init_ai(state);

	auto enemyBmp = renderer.load_sprite("enemy.psd");
	for (auto i = 0; i < 1; ++i) {
		auto entity = state.create();
		state.emplace<transform>(entity, math::vector2{ float(rand() % 1500), float(rand() % 1500) });
		state.emplace<sprite_instance>(entity, enemyBmp);
		state.emplace<ai_agent>(entity);
		state.emplace<inventory>(entity);
	}

	auto deerBmp = renderer.load_sprite("deer.psd");
	for (auto i = 0; i < 10; ++i) {
		auto entity = state.create();
		state.emplace<transform>(entity, math::vector2{ float(rand() % 1500), float(rand() % 1500) });
		state.emplace<sprite_instance>(entity, deerBmp);
		state.emplace<velocity>(entity, ((rand() * 12345 + 46789) % 30) * 0.1f, ((rand() * 546788784563 + 123456) % 30) * .1f, 0.0f);
		state.emplace<deer>(entity);
	}

	auto axeBmp = renderer.load_sprite("axe.psd");
	for (auto i = 0; i < 10; ++i) {
		auto entity = state.create();
		state.emplace<transform>(entity, math::vector2{ float(rand() % 1500), float(rand() % 1500) });
		state.emplace<sprite_instance>(entity, axeBmp);
		state.emplace<item>(entity, item_type::axe);
	}

	auto treeBmp = renderer.load_sprite("tree.psd");
	for (auto i = 0; i < 60; ++i) {
		auto entity = state.create();
		state.emplace<transform>(entity, math::vector2{ float(rand() % 1500), float(rand() % 1500) });
		state.emplace<sprite_instance>(entity, treeBmp);
		state.emplace<tree>(entity);
	}

	camera mainCamera{};
	mainCamera.position = { 0, 0 };
	mainCamera.angle = 0;
	state.set<camera>() = mainCamera;

	return state;
}


void update_physics(entt::registry& state) {
	for (auto e : state.group<transform, velocity>()) {
		auto& tr = state.get<transform>(e);
		auto& vel = state.get<velocity>(e);

		auto& pos = tr.position;
		pos.x += vel.dx;
		pos.y += vel.dy;
		if (pos.x < 0) { pos.x = 0; vel.dx *= -1; }
		if (pos.x > 1400) { pos.x = 1400; vel.dx *= -1; }
		if (pos.y < 0) { pos.y = 0; vel.dy *= -1; }
		if (pos.y > 1400) { pos.y = 1400; vel.dy *= -1; }

		tr.angle += vel.angular;
	}

	for (auto [e, velocity] : state.view<velocity>().each()) {
		velocity.dx *= 0.9f;
		velocity.dy *= 0.9f;
		velocity.angular *= 0.9f;
	}
}

void update_kill(entt::registry& state, renderer& renderer) {
	for (auto [e] : state.view<deer, kill>().each()) {
		auto dead_deer = renderer.load_sprite("dead_deer.psd");

		state.replace<sprite_instance>(e, dead_deer);
		state.remove<velocity, deer, kill>(e);

		auto sp = renderer.load_sprite("meat.psd");
		auto& tr = state.get<transform>(e);
		for (int i = 0; i < 3; ++i) {
			auto new_meat = state.create();
			state.emplace<transform>(new_meat, tr);
			state.emplace<sprite_instance>(new_meat, sp);
			state.emplace<item>(new_meat, item_type::meat);
			state.emplace<velocity>(new_meat, 0.4f * (rand() % 20 - 10), 0.4f * (rand() % 20 - 10), (rand() % 100 - 50) * 0.5f);
		}
	}
}

void update_commands(entt::registry& state) {
	auto& queue = state.ctx_or_set<command_queue>();

	while (!queue.commands.empty()) {
		auto command = queue.commands[queue.commands.size() - 1];
		queue.commands.pop_back();
		command->exec(state);
	}
}

void game_run_tick(entt::registry& state, engine& engine) {
	update_commands(state);

	auto& ps = state.ctx<global_state>();

	debug::draw_screen_text(state, { 100, 16 }, std::format("tick={}", ps.tick));

	if (ps.paused) {
		debug::draw_screen_text(state, { 200, 16 }, "(paused)");
		return;
	}

	++ps.tick;

	update_physics(state);
	update_ai(state);
	update_kill(state, engine.get_renderer());
}


void kill_deer(entt::registry& state, entt::entity target) {
	assert(state.all_of<deer>(target));
	state.emplace<kill>(target);
}

bool inventory_add_item(entt::registry& state, entt::entity inventory_owner, entt::entity new_item) {
	assert(state.all_of< item >(new_item));

	auto& inv = state.get<inventory>(inventory_owner);

	for (auto i = 0; i < inv.items.size(); ++i) {
		if (inv.items[i] == entt::null) {
			inv.items[i] = new_item;

			if (state.all_of<transform>(new_item)) {
				state.remove<transform>(new_item);
			}

			return true;
		}
	}
	return false;
}


bool inventory_has_item_of_type(entt::registry& state, entt::entity inventory_owner, item_type type) {
	auto& inv = state.get<inventory>(inventory_owner);

	for (auto i = 0; i < inv.items.size(); ++i) {
		if (inv.items[i] == entt::null)
			return false;

		auto it = state.get<item>(inv.items[i]);
		if (it.type == type)
			return true;
	}
	return false;
}


entt::entity inventory_remove_item_of_type(entt::registry& state, entt::entity inventory_owner, item_type type) {
	auto& inv = state.get<inventory>(inventory_owner);

	for (auto i = 0; i < inv.items.size(); ++i) {
		if (inv.items[i] == entt::null)
			return entt::null;

		auto it = state.get<item>(inv.items[i]);
		if (it.type == type) {
			auto item = inv.items[i];
			inv.items[i] = entt::null;
			return item;
		}
	}
	return entt::null;
}