#include "pch.h"
#include "game.h"
#include "ai.h"
#include "network_messages.h"
#include "game_network_adapter.h"
#include "app_states.h"

using namespace entt::literals;






void game_add_server_stuff(scene& scene, engine& engine) {
	auto& resource_manager = engine.get_resource_manager();



	auto enemy_bmp = resource_manager.load_bitmap_resource("enemy.psd"_hs);
	for (auto i = 0; i < 1; ++i) {
		auto entity = scene.create_entity();
		entity.emplace<component::transform>(math::vector2{ float(rand() % 1500), float(rand() % 1500) });
		entity.emplace<component::sprite_instance>(enemy_bmp);
		entity.emplace<component::ai_agent>();
		entity.emplace<component::inventory>();
	}

	auto deer_bmp = resource_manager.load_bitmap_resource("deer.psd"_hs);
	for (auto i = 0; i < 10; ++i) {
		auto entity = scene.create_entity();
		entity.emplace<component::transform>(math::vector2{ float(rand() % 1500), float(rand() % 1500) });
		entity.emplace<component::sprite_instance>(deer_bmp);
		entity.emplace<component::velocity>();
		entity.emplace<component::animal>();
	}

	auto axe_bmp = resource_manager.load_bitmap_resource("axe.psd"_hs);
	for (auto i = 0; i < 10; ++i) {
		auto entity = scene.create_entity();
		entity.emplace<component::transform>(math::vector2{ float(rand() % 1500), float(rand() % 1500) });
		entity.emplace<component::sprite_instance>(axe_bmp);
		entity.emplace<component::item>(item_type::axe);
	}

	auto tree_bmp = resource_manager.load_bitmap_resource("tree.psd"_hs);
	for (auto i = 0; i < 50; ++i) {
		auto entity = scene.create_entity();
		entity.emplace<component::transform>(math::vector2{ float(rand() % 1500), float(rand() % 1500) });
		entity.emplace<component::sprite_instance>(tree_bmp);
		entity.emplace<component::tree>();
	}

	auto grass_bmp = resource_manager.load_bitmap_resource("grass.psd"_hs);
	for (auto i = 0; i < 200; ++i) {
		auto entity = scene.create_entity();
		entity.emplace<component::transform>(math::vector2{ float(rand() % 1500), float(rand() % 1500) });
		entity.emplace<component::sprite_instance>(grass_bmp);
		entity.emplace<component::animal_edible>();
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
	for (auto [e, tr, vel] : scene.registry.group<component::transform, component::velocity>().each()) {
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

void update_kill(engine& engine, scene& scene) {
	for (auto [e, animal] : scene.registry.view<component::animal, component::kill>().each()) {
		auto dead_deer = engine.get_resource_manager().load_bitmap_resource("dead_deer.psd"_hs);

		scene.registry.replace<component::sprite_instance>(e, dead_deer);
		scene.registry.remove<component::velocity, component::animal, component::kill>(e);

		auto meat_bmp = engine.get_resource_manager().load_bitmap_resource("meat.psd"_hs);
		auto& tr = scene.registry.get<component::transform>(e);
		for (int i = 0; i < 3; ++i) {
			auto new_meat = scene.create_entity();
			new_meat.emplace<component::transform>(tr);
			new_meat.emplace<component::sprite_instance>(meat_bmp);
			new_meat.emplace<component::item>(item_type::meat);
			new_meat.emplace<component::velocity>(0.4f * (rand() % 20 - 10), 0.4f * (rand() % 20 - 10), (rand() % 100 - 50) * 0.5f);
		}
	}
}

void update_pawn_inputs(engine& engine, scene& scene) {
	for (auto [e, velo, inp] : scene.registry.view<component::velocity, component::pawn_tick_input>().each()) {
		velo.dx = (inp.tick_input.right - inp.tick_input.left) * 2;
		velo.dy = (inp.tick_input.down - inp.tick_input.up) * 2;

		auto& anim = scene.registry.get<component::animated_sprite>(e);
		if (inp.tick_input.up) {
			anim.animation_idx = 12;
		}
		else if (inp.tick_input.left) {
			anim.animation_idx = 13;
		}
		else if (inp.tick_input.down) {
			anim.animation_idx = 14;
		}
		else if (inp.tick_input.right) {
			anim.animation_idx = 15;
		}
		else {
			if (anim.animation_idx == 12) {
				anim.animation_idx = 0;
			}
			else if (anim.animation_idx == 13) {
				anim.animation_idx = 1;
			}
			else if (anim.animation_idx == 14) {
				anim.animation_idx = 2;
			}
			else if (anim.animation_idx == 15) {
				anim.animation_idx = 3;
			}
		}
	}

	for (auto [e, pawn, inp] : scene.registry.view<component::pawn, component::pawn_tick_input>().each()) {
		if (pawn.cooldown_ticks > 0) {
			--pawn.cooldown_ticks;
		}

		if (inp.tick_input.action0 && pawn.cooldown_ticks == 0) {
			pawn.cooldown_ticks = 20;

			auto spell_bmp = engine.get_resource_manager().load_bitmap_resource("spell.psd"_hs);

			auto& tr = scene.registry.get<component::transform>(e);

			auto new_spell = scene.create_entity();
			new_spell.emplace<component::transform>(tr);
			new_spell.emplace<component::sprite_instance>(spell_bmp);
			new_spell.emplace<component::velocity>(60.f, 0.f, 0.f);
		}
	}
}

void update_animal(scene& scene) {
	for (auto [e, tr, animal] : scene.registry.view<component::transform, component::animal>().each()) {
		if (animal.state == animal_state::idle) {
			entt::entity closest_grass = entt::null;
			float closest_dist = std::numeric_limits<float>::max();

			for (auto [e2, tr2] : scene.registry.view<component::transform, component::animal_edible>().each()) {
				auto dist = (tr.position - tr2.position).sqr_magnitude();
				if (dist < closest_dist) {
					closest_dist = dist;
					closest_grass = e2;
				}
			}

			if (closest_grass == entt::null)
				continue;

			animal.current_target = closest_grass;
			animal.state = animal_state::going_to_grass;
		}
		else if (animal.state == animal_state::going_to_grass) {
			if (!scene.registry.valid(animal.current_target)) {
				animal.state = animal_state::idle;
				continue;
			}

			auto target_tr = scene.registry.get<component::transform>(animal.current_target);

			auto diff = target_tr.position - tr.position;
			if (diff.sqr_magnitude() > 0.6f) {
				diff = diff.normalised() * 1.5f;

				auto& vel = scene.registry.get<component::velocity>(e);
				vel.dx = diff.x;
				vel.dy = diff.y;
			}
			else {
				scene.registry.destroy(animal.current_target);
				animal.state = animal_state::eating_grass;
				animal.eating_ticks = 1000;
			}
		}
		else if (animal.state == animal_state::eating_grass) {
			if (animal.eating_ticks > 0) {
				--animal.eating_ticks;
			}
			else {
				animal.state = animal_state::idle;
			}
		}
	}
}

size_t game_get_tick(scene& scene) {
	auto& ps = scene.registry.ctx<component::global_state>();
	return ps.tick;
}

void game_run_tick(scene& scene, engine& engine) {
	ZoneScoped;

	auto& ps = scene.registry.ctx<component::global_state>();
	++ps.tick;

	debug::draw_screen_text_at(scene, { 300, 16 }, std::format("tick={} local_client_idx={}", ps.tick, ps.local_client_idx));

	update_pawn_inputs(engine, scene);
	update_animal(scene);
	update_ai(scene);
	update_kill(engine, scene);
	update_physics(scene);
}


void kill_deer(scene& scene, entt::entity target) {
	fabrik_assert(scene.registry.all_of<component::animal>(target));
	scene.registry.emplace<component::kill>(target);
}

bool inventory_add_item(scene& scene, entt::entity inventory_owner, entt::entity new_item) {
	fabrik_assert(scene.registry.all_of<component::item>(new_item));

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
	auto player_bmp = engine.get_resource_manager().load_bitmap_resource("wizard.png"_hs);
	auto anim = engine.get_resource_manager().load_anim_resource("character.anim"_hs);

	auto entity = scene.create_entity();
	entity.emplace<component::transform>(math::vector2{ 800, 800 });
	entity.emplace<component::sprite_instance>(player_bmp);
	entity.emplace<component::velocity>();
	entity.emplace<component::pawn>(client_idx);
	entity.emplace<component::animated_sprite>(anim);
}

void send_initial_game_state(engine& engine, scene& scene, int client_idx) {
	std::string str;
	{
		std::stringstream ss;
		{
			cereal::JSONOutputArchive output{ ss };
			entt::snapshot{ scene.registry }
				.entities(output)
				.component<component::transform>(output)
				.component<component::velocity>(output)
				.component<component::pawn>(output)
				.component<component::tree>(output)
				.component<component::animal_edible>(output)
				.component<component::animal>(output)
				.component<component::item>(output)
				.component<component::sprite_instance>(output)
				.component<component::ai_agent>(output)
				.component<component::inventory>(output)
				.component<component::animated_sprite>(output)
				;
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

	std::stringstream ss{ data };
	cereal::JSONInputArchive input{ ss };
	entt::snapshot_loader{ scene.registry }
		.entities(input)
		.component<component::transform>(input)
		.component<component::velocity>(input)
		.component<component::pawn>(input)
		.component<component::tree>(input)
		.component<component::animal_edible>(input)
		.component<component::animal>(input)
		.component<component::item>(input)
		.component<component::sprite_instance>(input)
		.component<component::ai_agent>(input)
		.component<component::inventory>(input)
		.component<component::animated_sprite>(input)
		;

	// Patch resources
	for (auto [e, sprite] : scene.registry.view<component::sprite_instance>().each()) {
		const auto path = engine.get_resource_manager().lookup_resource_path_by_hash(sprite.bitmap.resource_hash);
		if (!path)
			panic("Resource from server not found");

		sprite.bitmap.runtime_idx = engine.get_resource_manager().load_bitmap_resource(entt::hashed_string{ path->c_str() }).runtime_idx;
	}
	for (auto [e, sprite] : scene.registry.view<component::animated_sprite>().each()) {
		const auto path = engine.get_resource_manager().lookup_resource_path_by_hash(sprite.spritesheet.resource_hash);
		if (!path)
			panic("Resource from server not found");

		sprite.spritesheet.runtime_idx = engine.get_resource_manager().load_anim_resource(entt::hashed_string{ path->c_str() }).runtime_idx;
	}


	// Patch global state
	auto& ps = scene.registry.ctx<component::global_state>();
	ps.tick = message->tick;
	ps.local_client_idx = message->local_client_idx;

	engine.get_app_state_manager().switch_to<app_states::client_ingame>();
}