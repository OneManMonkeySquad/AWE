#include "pch.h"
#include "game.h"
#include "ai.h"
#include "network_messages.h"
#include "game_network_adapter.h"
#include "app_states.h"

using namespace entt::literals;






void game_add_server_stuff(scene& scene, engine& engine) {
	auto& resource_manager = engine.get_resource_manager();



	auto enemyBmp = resource_manager.load_bitmap_resource("enemy.psd");
	for (auto i = 0; i < 1; ++i) {
		auto entity = scene.create_entity();
		entity.emplace<component::transform>(math::vector2{ float(rand() % 1500), float(rand() % 1500) });
		entity.emplace<component::sprite_instance>(enemyBmp);
		entity.emplace<component::ai_agent>();
		entity.emplace<component::inventory>();
	}

	auto deerBmp = resource_manager.load_bitmap_resource("deer.psd");
	for (auto i = 0; i < 10; ++i) {
		auto entity = scene.create_entity();
		entity.emplace<component::transform>(math::vector2{ float(rand() % 1500), float(rand() % 1500) });
		entity.emplace<component::sprite_instance>(deerBmp);
		entity.emplace<component::velocity>(((rand() * 12345 + 46789) % 30) * 0.1f, ((rand() * 546788784563 + 123456) % 30) * .1f, 0.0f);
		entity.emplace<component::deer>();
	}

	auto axeBmp = resource_manager.load_bitmap_resource("axe.psd");
	for (auto i = 0; i < 10; ++i) {
		auto entity = scene.create_entity();
		entity.emplace<component::transform>(math::vector2{ float(rand() % 1500), float(rand() % 1500) });
		entity.emplace<component::sprite_instance>(axeBmp);
		entity.emplace<component::item>(item_type::axe);
	}

	auto treeBmp = resource_manager.load_bitmap_resource("tree.psd");
	for (auto i = 0; i < 60; ++i) {
		auto entity = scene.create_entity();
		entity.emplace<component::transform>(math::vector2{ float(rand() % 1500), float(rand() % 1500) });
		entity.emplace<component::sprite_instance>(treeBmp);
		entity.emplace<component::tree>();
	}

	auto grass_bmp = resource_manager.load_bitmap_resource("grass.psd"_hs);
	for (auto i = 0; i < 200; ++i) {
		auto entity = scene.create_entity();
		entity.emplace<component::transform>(math::vector2{ float(rand() % 1500), float(rand() % 1500) });
		entity.emplace<component::sprite_instance>(grass_bmp);
		entity.emplace<component::grass>();
	}
}

scene& game_create(engine& engine) {
	auto& scene = engine.get_scene_manager().create_scene();

	scene.registry.set<component::global_state>();

	init_ai(scene);

	component::camera mainCamera{};
	mainCamera.position = { 0, 0 };
	mainCamera.angle = 0;
	scene.registry.set<component::camera>() = mainCamera;

	return scene;
}


void update_physics(scene& scene) {
	for (auto e : scene.registry.group<component::transform, component::velocity>()) {
		auto& tr = scene.registry.get<component::transform>(e);
		auto& vel = scene.registry.get<component::velocity>(e);

		auto& pos = tr.position;
		pos.x += vel.dx;
		pos.y += vel.dy;
		if (pos.x < 0) { pos.x = 0; vel.dx *= -1; }
		if (pos.x > 1400) { pos.x = 1400; vel.dx *= -1; }
		if (pos.y < 0) { pos.y = 0; vel.dy *= -1; }
		if (pos.y > 1400) { pos.y = 1400; vel.dy *= -1; }

		tr.angle += vel.angular;
	}

	for (auto [e, velocity] : scene.registry.view<component::velocity>().each()) {
		velocity.dx *= 0.9f;
		velocity.dy *= 0.9f;
		velocity.angular *= 0.9f;
	}
}

void update_kill(scene& scene, engine& engine) {
	for (auto [e] : scene.registry.view<component::deer, component::kill>().each()) {
		auto dead_deer = engine.get_resource_manager().load_bitmap_resource("dead_deer.psd");

		scene.registry.replace<component::sprite_instance>(e, dead_deer);
		scene.registry.remove<component::velocity, component::deer, component::kill>(e);

		auto sp = engine.get_resource_manager().load_bitmap_resource("meat.psd");
		auto& tr = scene.registry.get<component::transform>(e);
		for (int i = 0; i < 3; ++i) {
			auto new_meat = scene.create_entity();
			new_meat.emplace<component::transform>(tr);
			new_meat.emplace<component::sprite_instance>(sp);
			new_meat.emplace<component::item>(item_type::meat);
			new_meat.emplace<component::velocity>(0.4f * (rand() % 20 - 10), 0.4f * (rand() % 20 - 10), (rand() % 100 - 50) * 0.5f);
		}
	}
}

void update_pawn_inputs(scene& scene) {
	for (auto [e, velo, inp] : scene.registry.view<component::velocity, component::pawn_tick_input>().each()) {
		velo.dx = (inp.tick_input.right - inp.tick_input.left) * 4;
		velo.dy = (inp.tick_input.down - inp.tick_input.up) * 4;
	}
}

size_t game_get_tick(scene& scene) {
	auto& ps = scene.registry.ctx<component::global_state>();
	return ps.tick;
}

void game_run_tick(scene& scene, engine& engine) {
	ZoneScoped;

	auto& ps = scene.registry.ctx<component::global_state>();
	debug::draw_screen_text_at(scene, { 300, 16 }, std::format("tick={} local_client_idx={}", ps.tick, ps.local_client_idx));
	++ps.tick;

	update_pawn_inputs(scene);
	update_physics(scene);
	update_ai(scene);
	update_kill(scene, engine);
}


void kill_deer(scene& scene, entt::entity target) {
	fabrik_assert(scene.registry.all_of<component::deer>(target));
	scene.registry.emplace<component::kill>(target);
}

bool inventory_add_item(scene& scene, entt::entity inventory_owner, entt::entity new_item) {
	fabrik_assert(scene.registry.all_of< item >(new_item));

	auto& inv = scene.registry.get<component::inventory>(inventory_owner);

	for (auto i = 0; i < inv.items.size(); ++i) {
		if (inv.items[i] == entt::null) {
			inv.items[i] = new_item;

			if (scene.registry.all_of<component::transform>(new_item)) {
				scene.registry.remove<component::transform>(new_item);
			}

			return true;
		}
	}
	return false;
}


bool inventory_has_item_of_type(scene& scene, entt::entity inventory_owner, item_type type) {
	auto& inv = scene.registry.get<component::inventory>(inventory_owner);

	for (auto i = 0; i < inv.items.size(); ++i) {
		if (inv.items[i] == entt::null)
			return false;

		auto it = scene.registry.get<component::item>(inv.items[i]);
		if (it.type == type)
			return true;
	}
	return false;
}


entt::entity inventory_remove_item_of_type(scene& scene, entt::entity inventory_owner, item_type type) {
	auto& inv = scene.registry.get<component::inventory>(inventory_owner);

	for (auto i = 0; i < inv.items.size(); ++i) {
		if (inv.items[i] == entt::null)
			return entt::null;

		auto it = scene.registry.get<component::item>(inv.items[i]);
		if (it.type == type) {
			auto item = inv.items[i];
			inv.items[i] = entt::null;
			return item;
		}
	}
	return entt::null;
}



void create_player(engine& engine, scene& scene, int client_idx) {
	auto enemyBmp = engine.get_resource_manager().load_bitmap_resource("player.psd");

	auto entity = scene.create_entity();
	entity.emplace<component::transform>(math::vector2{ 800, 800 });
	entity.emplace<component::sprite_instance>(enemyBmp);
	entity.emplace<component::velocity>();
	entity.emplace<component::pawn>(client_idx);
}

void send_initial_game_state(engine& engine, scene& scene, int client_idx) {
	std::string str;
	{
		std::stringstream ss;
		{
			cereal::JSONOutputArchive output{ ss };
			entt::snapshot{ scene.registry }
				.entities(output)
				.component<component::transform, component::velocity, component::pawn, component::tree, component::deer, component::item, component::sprite_instance, component::ai_agent, component::inventory>(output);
		}
		str = ss.str();
	}

	const auto packet_size = 1024;
	auto num = (str.size() / packet_size) + 1;
	for (int i = 0; i < num; ++i) {
		auto msg = (network_messages::initial_game_state*)engine.get_network().get_server().CreateMessage(client_idx, INITIAL_GAME_STATE_MESSAGE);
		msg->data = str.substr(i * packet_size, packet_size);

		engine.get_network().get_server().SendMessage(client_idx, 0, msg);
	}

	{
		auto& ps = scene.registry.ctx<component::global_state>();

		auto msg = (network_messages::initial_game_state_end*)engine.get_network().get_server().CreateMessage(client_idx, INITIAL_GAME_STATE_END_MESSAGE);
		msg->data_hash = std::hash<std::string>()(str);
		msg->tick = ps.tick;
		msg->local_client_idx = client_idx;
		engine.get_network().get_server().SendMessage(client_idx, 0, msg);
	}
}

void on_initial_game_state(engine& engine, scene& scene, const std::string& data, network_messages::initial_game_state_end* message) {
	const auto hash = std::hash<std::string>()(data);
	if (hash != ((network_messages::initial_game_state_end*)message)->data_hash) {
		engine.get_app_state_manager().switch_to<app_states::client_disconnected>("Initial Game State Corrupt");
		return;
	}

	scene.clear();

	print("HANDLING INITIAL STATE");

	std::stringstream ss{ data };
	cereal::JSONInputArchive input{ ss };
	entt::snapshot_loader{ scene.registry }
		.entities(input)
		.component<component::transform, component::velocity, component::pawn, component::tree, component::deer, component::item, component::sprite_instance, component::ai_agent, component::inventory>(input);

	// Patch resources
	for (auto [e, sprite] : scene.registry.view<component::sprite_instance>().each()) {
		const auto path = engine.get_resource_manager().lookup_resource_path_by_hash(sprite.bitmap.resource_hash);
		if (!path)
			panic("Resource from server not found");

		sprite.bitmap.runtime_idx = engine.get_resource_manager().load_bitmap_resource(entt::hashed_string{ path->c_str() }).runtime_idx;
	}

	// Patch global state
	auto& ps = scene.registry.ctx<component::global_state>();
	ps.tick = message->tick;
	ps.local_client_idx = message->local_client_idx;

	engine.get_app_state_manager().switch_to<app_states::client_ingame>();
}