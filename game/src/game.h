
#pragma once

class engine;
namespace network_messages {
	struct initial_game_state_end;
}

struct game_input {
	bool left;
	bool right;
	bool up;
	bool down;
	// LMB or A
	bool action0;

	template <typename Stream>
	bool Serialize(Stream& stream) {
		serialize_bool(stream, left);
		serialize_bool(stream, right);
		serialize_bool(stream, up);
		serialize_bool(stream, down);
		serialize_bool(stream, action0);
		return true;
	}
};

enum class item_type { axe, meat, wood };

enum class animal_state : uint8_t {
	idle,
	going_to_grass,
	eating_grass
};

namespace component {
	struct global_state {
		size_t tick = 0;
		int local_client_idx = -1;
	};

	struct velocity {
		float dx = 0;
		float dy = 0;
		float angular = 0;

		template<typename Archive>
		void serialize(Archive& archive) {
			archive(dx, dy, angular);
		}
	};

	struct item {
		item_type type;

		template<typename Archive>
		void serialize(Archive& archive) {
			archive((int&)type);
		}
	};

	struct inventory {
		std::array<entt::entity, 3> items = { entt::null, entt::null, entt::null };

		template<typename Archive>
		void serialize(Archive& archive) {
			for (int i = 0; i < items.size(); ++i) {
				archive(items[i]);
			}
		}
	};

	struct tree {};
	struct animal_edible {};
	struct animal {
		entt::entity current_target;
		animal_state state;
		uint32_t eating_ticks;

		template<typename Archive>
		void serialize(Archive& archive) {
			archive(state);
		}
	};
	struct kill {};

	struct pawn {
		int client_idx = -1;
		uint32_t cooldown_ticks = 0;

		template<typename Archive>
		void serialize(Archive& archive) {
			archive(client_idx, cooldown_ticks);
		}
	};

	struct pawn_tick_input {
		game_input tick_input;
	};
}

scene& game_create(engine& engine);
void game_add_server_stuff(scene& scene, engine& engine);

size_t game_get_tick(scene& scene);
void game_run_tick(scene& scene, engine& engine);

void kill_deer(scene& scene, entt::entity target);

bool inventory_add_item(scene& scene, entt::entity inventory_owner, entt::entity new_item);
bool inventory_has_item_of_type(scene& scene, entt::entity inventory_owner, item_type type);
entt::entity inventory_remove_item_of_type(scene& scene, entt::entity inventory_owner, item_type type);



void create_player(engine& engine, scene& scene, int client_idx);
void send_initial_game_state(engine& engine, scene& scene, int client_idx);
void on_initial_game_state(engine& engine, scene& scene, const std::string& data, network_messages::initial_game_state_end* message);

