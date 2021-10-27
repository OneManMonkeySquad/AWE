
#pragma once

#include "game.h"

namespace app_states {
	class client_download_scene;

	class server_host : public app_state {
	public:
		server_host(engine& engine);

		void enter();
		void exit();

		void begin_frame();
		void tick();

		void on_event(const events::client_connected& evt);
		void on_event(const events::client_disconnected& evt);

	private:
		game_input _inputs[yojimbo::MaxClients] = {};
	};

	class client_disconnected : public app_state {
	public:
		client_disconnected(engine& engine, std::string reason)
			: app_state(engine) {}

		void enter() {}
		void exit() {}

		void begin_frame() {}
		void tick() {}
	};

	class client_connect : public app_state {
	public:
		client_connect(engine& engine);

		void enter();
		void exit();

		void begin_frame();
		void tick() {}
	};

	class client_download_scene : public app_state {
	public:
		client_download_scene(engine& engine);

		void enter();
		void exit();

		void begin_frame();
		void tick();

	private:
		std::string _initial_game_state_buffer;
	};

	class client_ingame : public app_state {
	public:
		client_ingame(engine& engine);

		void enter();
		void exit();

		void begin_frame();
		void tick();

	private:
		std::chrono::nanoseconds _game_lag;
		std::chrono::high_resolution_clock::time_point _time_start;

		entt::registry _previous_rendable_registry;

		game_input _current_input;

		void update_camera(scene& scene);

		game_input sample_current_game_input();

		void send_tick_input(size_t tick, game_input input);
	};
}