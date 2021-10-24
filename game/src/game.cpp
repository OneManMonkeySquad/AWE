#include "pch.h"
#include "game.h"
#include "ai.h"
#include "network_messages.h"
#include "game_network_adapter.h"

class command {
public:
	virtual ~command() = default;

	virtual void exec(scene& scene) = 0;
};

class pause_command : public command {
public:
	void exec(scene& scene) {
		auto& ps = scene.registry.ctx<global_state>();
		ps.paused = !ps.paused;
	}
};

class pawn_input_command : public command {
public:
	math::vector2 _dir;

	pawn_input_command(math::vector2 dir) : _dir(dir) {}

	void exec(scene& scene) {
		for (auto [e, velo] : scene.registry.view<pawn, velocity>().each()) {
			velo.dx = _dir.x * 8;
			velo.dy = _dir.y * 8;
		}
	}
};


struct command_queue {
	std::vector<std::shared_ptr<command>> commands;
};


void game_create_commands_from_input(scene& scene, input& input) {
	auto& queue = scene.registry.ctx_or_set<command_queue>();

	for (auto& evt : input.events) {
		if (evt.type == input_event_type::key_down && evt.key == ALLEGRO_KEY_SPACE) {
			queue.commands.push_back(std::make_shared<pause_command>());
		}
	}

	const auto w = input.is_key_down(ALLEGRO_KEY_W);
	const auto s = input.is_key_down(ALLEGRO_KEY_S);
	const auto a = input.is_key_down(ALLEGRO_KEY_A);
	const auto d = input.is_key_down(ALLEGRO_KEY_D);

	math::vector2 offset;
	offset.x = (d - a);
	offset.y = (s - w);

	queue.commands.push_back(std::make_shared<pawn_input_command>(offset));
}


void game_add_server_stuff(scene& scene, engine& engine) {
	auto& resource_manager = engine.get_resource_manager();



	auto enemyBmp = resource_manager.load_bitmap_resource("enemy.psd");
	for (auto i = 0; i < 1; ++i) {
		auto entity = scene.create_entity();
		entity.emplace<transform>(math::vector2{ float(rand() % 1500), float(rand() % 1500) });
		entity.emplace<sprite_instance>(enemyBmp);
		entity.emplace<ai_agent>();
		entity.emplace<inventory>();
	}

	auto deerBmp = resource_manager.load_bitmap_resource("deer.psd");
	for (auto i = 0; i < 10; ++i) {
		auto entity = scene.create_entity();
		entity.emplace<transform>(math::vector2{ float(rand() % 1500), float(rand() % 1500) });
		entity.emplace<sprite_instance>(deerBmp);
		entity.emplace<velocity>(((rand() * 12345 + 46789) % 30) * 0.1f, ((rand() * 546788784563 + 123456) % 30) * .1f, 0.0f);
		entity.emplace<deer>();
	}

	auto axeBmp = resource_manager.load_bitmap_resource("axe.psd");
	for (auto i = 0; i < 10; ++i) {
		auto entity = scene.create_entity();
		entity.emplace<transform>(math::vector2{ float(rand() % 1500), float(rand() % 1500) });
		entity.emplace<sprite_instance>(axeBmp);
		entity.emplace<item>(item_type::axe);
	}

	auto treeBmp = resource_manager.load_bitmap_resource("tree.psd");
	for (auto i = 0; i < 60; ++i) {
		auto entity = scene.create_entity();
		entity.emplace<transform>(math::vector2{ float(rand() % 1500), float(rand() % 1500) });
		entity.emplace<sprite_instance>(treeBmp);
		entity.emplace<tree>();
	}
}

scene& game_create(engine& engine) {
	auto& scene = engine.get_scene_manager().create_scene();

	scene.registry.set<global_state>();

	init_ai(scene);

	camera mainCamera{};
	mainCamera.position = { 0, 0 };
	mainCamera.angle = 0;
	scene.registry.set<camera>() = mainCamera;

	return scene;
}


void update_physics(scene& scene) {
	for (auto e : scene.registry.group<transform, velocity>()) {
		auto& tr = scene.registry.get<transform>(e);
		auto& vel = scene.registry.get<velocity>(e);

		auto& pos = tr.position;
		pos.x += vel.dx;
		pos.y += vel.dy;
		if (pos.x < 0) { pos.x = 0; vel.dx *= -1; }
		if (pos.x > 1400) { pos.x = 1400; vel.dx *= -1; }
		if (pos.y < 0) { pos.y = 0; vel.dy *= -1; }
		if (pos.y > 1400) { pos.y = 1400; vel.dy *= -1; }

		tr.angle += vel.angular;
	}

	for (auto [e, velocity] : scene.registry.view<velocity>().each()) {
		velocity.dx *= 0.9f;
		velocity.dy *= 0.9f;
		velocity.angular *= 0.9f;
	}
}

void update_kill(scene& scene, engine& engine) {
	for (auto [e] : scene.registry.view<deer, kill>().each()) {
		auto dead_deer = engine.get_resource_manager().load_bitmap_resource("dead_deer.psd");

		scene.registry.replace<sprite_instance>(e, dead_deer);
		scene.registry.remove<velocity, deer, kill>(e);

		auto sp = engine.get_resource_manager().load_bitmap_resource("meat.psd");
		auto& tr = scene.registry.get<transform>(e);
		for (int i = 0; i < 3; ++i) {
			auto new_meat = scene.create_entity();
			new_meat.emplace<transform>(tr);
			new_meat.emplace<sprite_instance>(sp);
			new_meat.emplace<item>(item_type::meat);
			new_meat.emplace<velocity>(0.4f * (rand() % 20 - 10), 0.4f * (rand() % 20 - 10), (rand() % 100 - 50) * 0.5f);
		}
	}
}

void update_commands(scene& scene) {
	auto& queue = scene.registry.ctx_or_set<command_queue>();

	while (!queue.commands.empty()) {
		auto command = queue.commands[queue.commands.size() - 1];
		queue.commands.pop_back();
		command->exec(scene);
	}
}

void game_run_tick(scene& scene, engine& engine) {
	ZoneScoped;

	update_commands(scene);

	auto& ps = scene.registry.ctx<global_state>();

	debug::draw_screen_text_at(scene, { 300, 16 }, std::format("tick={}", ps.tick));

	if (ps.paused) {
		debug::draw_screen_text_at(scene, { 400, 16 }, "(paused)");
		return;
	}

	++ps.tick;

	update_physics(scene);
	update_ai(scene);
	update_kill(scene, engine);
}


void kill_deer(scene& scene, entt::entity target) {
	fabrik_assert(scene.registry.all_of<deer>(target));
	scene.registry.emplace<kill>(target);
}

bool inventory_add_item(scene& scene, entt::entity inventory_owner, entt::entity new_item) {
	fabrik_assert(scene.registry.all_of< item >(new_item));

	auto& inv = scene.registry.get<inventory>(inventory_owner);

	for (auto i = 0; i < inv.items.size(); ++i) {
		if (inv.items[i] == entt::null) {
			inv.items[i] = new_item;

			if (scene.registry.all_of<transform>(new_item)) {
				scene.registry.remove<transform>(new_item);
			}

			return true;
		}
	}
	return false;
}


bool inventory_has_item_of_type(scene& scene, entt::entity inventory_owner, item_type type) {
	auto& inv = scene.registry.get<inventory>(inventory_owner);

	for (auto i = 0; i < inv.items.size(); ++i) {
		if (inv.items[i] == entt::null)
			return false;

		auto it = scene.registry.get<item>(inv.items[i]);
		if (it.type == type)
			return true;
	}
	return false;
}


entt::entity inventory_remove_item_of_type(scene& scene, entt::entity inventory_owner, item_type type) {
	auto& inv = scene.registry.get<inventory>(inventory_owner);

	for (auto i = 0; i < inv.items.size(); ++i) {
		if (inv.items[i] == entt::null)
			return entt::null;

		auto it = scene.registry.get<item>(inv.items[i]);
		if (it.type == type) {
			auto item = inv.items[i];
			inv.items[i] = entt::null;
			return item;
		}
	}
	return entt::null;
}



void add_player(engine& engine, scene& scene) {
	auto enemyBmp = engine.get_resource_manager().load_bitmap_resource("player.psd");
	auto entity = scene.create_entity();
	entity.emplace<transform>(math::vector2{ float(rand() % 1500), float(rand() % 1500) });
	entity.emplace<sprite_instance>(enemyBmp);
	entity.emplace<velocity>();
	entity.emplace<pawn>();
}

void send_initial_game_state(engine& engine, scene& scene, int client_idx) {
	std::string str;
	{
		std::stringstream ss;
		{
			cereal::JSONOutputArchive output{ ss };
			entt::snapshot{ scene.registry }
				.entities(output)
				.component<transform, velocity, pawn, tree, deer, item, sprite_instance, ai_agent, inventory>(output);
		}
		str = ss.str();
	}

	const auto packet_size = 1024;
	auto num = (str.size() / packet_size) + 1;
	for (int i = 0; i < num; ++i) {
		auto msg = (network_messages::initial_game_state*)engine.get_network().get_server().CreateMessage(client_idx, INITIAL_GAME_STATE_MESSAGE);
		msg->data = str.substr(i * packet_size, packet_size);

		engine.get_network().get_server().SendMessage(client_idx, 0, msg);
		print("SERVER SEND MESSAGE TO CLIENT {}", i);
	}

	{
		auto msg = (network_messages::initial_game_state_end*)engine.get_network().get_server().CreateMessage(client_idx, INITIAL_GAME_STATE_END_MESSAGE);
		msg->data_hash = std::hash<std::string>()(str);
		engine.get_network().get_server().SendMessage(client_idx, 0, msg);
		print("SERVER SEND MESSAGE TO CLIENT END");
	}
}

void on_initial_game_state(engine& engine, scene& scene, const std::string& data) {
	scene.clear();

	print("HANDLING INITIAL STATE");

	std::stringstream ss{ data };
	cereal::JSONInputArchive input{ ss };
	entt::snapshot_loader{ scene.registry }
		.entities(input)
		.component<transform, velocity, pawn, tree, deer, item, sprite_instance, ai_agent, inventory>(input);

	// Patch resources
	for (auto [e, sprite] : scene.registry.view<sprite_instance>().each()) {
		const auto path = engine.get_resource_manager().lookup_resource_path_by_hash(sprite.bitmap.resource_hash);
		if (!path)
			panic("Resource from server not found");

		sprite.bitmap.runtime_idx = engine.get_resource_manager().load_bitmap_resource(entt::hashed_string{ path->c_str() }).runtime_idx;
	}
}