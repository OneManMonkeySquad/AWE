#pragma once

class engine;
class input;
namespace network_messages {
	struct initial_game_state_end;
}

struct game_input {
	bool left;
	bool right;
	bool up;
	bool down;
};

struct command_queue {
	std::vector<game_input> commands;
};



struct global_state {
	size_t tick = 0;
	int local_client_idx = -1;
};

struct velocity {
	float dx;
	float dy;
	float angular;

	template<typename Archive>
	void serialize(Archive& archive) {
		archive(dx, dy, angular);
	}
};

enum class item_type { axe, meat, wood };

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
struct deer {};
struct kill {};
struct dead_deer {};

struct pawn {
	int client_idx;

	template<typename Archive>
	void serialize(Archive& archive) {
		archive(client_idx);
	}
};

struct pawn_tick_input {
	game_input tick_input;
};

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

