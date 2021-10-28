#include "pch.h"
#include "app_states.h"
#include "game.h"
#include "game_network_adapter.h"

using namespace std::literals::chrono_literals;

namespace app_states {
	constexpr std::chrono::nanoseconds game_timestep = 16ms;
	constexpr std::chrono::nanoseconds max_single_timestep = 10ms;





	server_host::server_host(engine& engine)
		: app_state(engine) {}

	void server_host::enter() {
		auto& scene = _engine.get_scene_manager().get_first_scene();
		game_add_server_stuff(scene, _engine);

		_engine.get_network().start_server(std::make_unique<game_network_adapter>(_engine), 60000);

		_engine.get_event_hub().add_listener_instance<events::client_connected>(this);
		_engine.get_event_hub().add_listener_instance<events::client_disconnected>(this);
	}

	void server_host::exit() {}

	void server_host::begin_frame() {
		for (auto message : _engine.get_network().server_messages) {
			switch (message.second->GetType()) {
			case (int)TICK_INPUT_MESSAGE:
			{
				auto msg = (network_messages::tick_input*)message.second;

				_inputs[message.first] = msg->input;
				break;
			}
			}
		}
	}

	void server_host::tick() {
		auto& scene = _engine.get_scene_manager().get_first_scene();

		for (auto [e, pawn] : scene.registry.view<component::pawn>().each()) {
			auto& tick_input = scene.registry.get_or_emplace<component::pawn_tick_input>(e);
			tick_input.tick_input = _inputs[pawn.client_idx];
		}

		game_run_tick(scene, _engine);

		for (int client_idx = 0; client_idx < yojimbo::MaxClients; ++client_idx) {
			if (!_engine.get_network().get_server().IsClientConnected(client_idx))
				continue;

			auto msg = (network_messages::inputs*)_engine.get_network().get_server().CreateMessage(client_idx, TestMessageType::INPUTS_MESSAGE);

			for (int j = 0; j < yojimbo::MaxClients; ++j) {
				if (j == client_idx || !_engine.get_network().get_server().IsClientConnected(j))
					continue;

				msg->input_client_indices.push_back(j);
				msg->inputs.push_back(_inputs[j]);
			}

			_engine.get_network().get_server().SendMessage(client_idx, 0, msg);
		}
	}

	void server_host::on_event(const events::client_connected& evt) {
		auto& scene = _engine.get_scene_manager().get_first_scene();

		{
			for (int client_idx = 0; client_idx < yojimbo::MaxClients; ++client_idx) {
				if (client_idx == evt.client_idx || !_engine.get_network().get_server().IsClientConnected(client_idx))
					continue;

				print("Send create_player to {}", client_idx);

				auto msg = (network_messages::create_player*)_engine.get_network().get_server().CreateMessage(client_idx, TestMessageType::CREATE_PLAYER_MESSAGE);
				msg->client_idx = evt.client_idx;
				_engine.get_network().get_server().SendMessage(client_idx, 0, msg);
			}
		}
		create_player(_engine, scene, evt.client_idx);

		print("Sending initial game state");
		send_initial_game_state(_engine, scene, evt.client_idx);
	}

	void server_host::on_event(const events::client_disconnected& evt) {}









	client_connect::client_connect(engine& engine)
		: app_state(engine) {}

	void client_connect::enter() {
		_engine.get_network().start_client(std::make_unique<game_network_adapter>(_engine));
		_engine.get_network().connect("127.0.0.1", 60000);
	}
	void client_connect::exit() {}
	void client_connect::begin_frame() {
		if (_engine.get_network().get_client().IsConnected()) {
			_engine.get_app_state_manager().switch_to<client_download_scene>();
		}
	}








	client_download_scene::client_download_scene(engine& engine)
		: app_state(engine) {}

	void client_download_scene::enter() {}
	void client_download_scene::exit() {}
	void client_download_scene::begin_frame() {
		if (!_engine.get_network().get_client().IsConnected()) {
			_engine.get_app_state_manager().switch_to<client_disconnected>("Connection Lost");
			return;
		}

		auto& scene = _engine.get_scene_manager().get_first_scene();

		for (auto message : _engine.get_network().client_messages) {
			switch (message->GetType()) {
			case (int)TestMessageType::INITIAL_GAME_STATE_MESSAGE:
				_initial_game_state_buffer += ((network_messages::initial_game_state*)message)->data;
				break;
			case (int)TestMessageType::INITIAL_GAME_STATE_END_MESSAGE:
				on_initial_game_state(_engine, scene, _initial_game_state_buffer, (network_messages::initial_game_state_end*)message);
				_initial_game_state_buffer.clear();
				break;
			}
		}
	}
	void client_download_scene::tick() {}









	client_ingame::client_ingame(engine& engine)
		: app_state(engine) {}

	void client_ingame::enter() {
		_game_lag = 0ns;
		_time_start = std::chrono::high_resolution_clock::now();
	}

	void client_ingame::exit() {}

	void client_ingame::begin_frame() {
		const auto delta_time = std::min(std::chrono::high_resolution_clock::now() - _time_start, max_single_timestep);
		_time_start = std::chrono::high_resolution_clock::now();
		_game_lag += duration_cast<std::chrono::nanoseconds>(delta_time);

		_current_input = sample_current_game_input();

		auto& scene = _engine.get_scene_manager().get_first_scene();
		for (auto message : _engine.get_network().client_messages) {
			switch (message->GetType()) {
			case (int)INPUTS_MESSAGE:
			{
				auto inputs = (network_messages::inputs*)message;


				auto num = inputs->input_client_indices.size();
				for (int i = 0; i < num; ++i) {
					const auto client_idx = inputs->input_client_indices[i];
					const auto input = inputs->inputs[i];

					// O(N^2)
					for (auto [e, pawn] : scene.registry.view<component::pawn>().each()) {
						if (pawn.client_idx != client_idx)
							continue;

						auto& tick_input = scene.registry.get_or_emplace<component::pawn_tick_input>(e);
						tick_input.tick_input = input;
					}
				}
				break;
			}
			case (int)TestMessageType::CREATE_PLAYER_MESSAGE:
				print("Client got create_player");

				create_player(_engine, scene, ((network_messages::create_player*)message)->client_idx);
				break;
			}
		}
	}

	void client_ingame::tick() {
		auto& scene = _engine.get_scene_manager().get_first_scene();

		auto current_tick = game_get_tick(scene);
		send_tick_input(current_tick, _current_input);

		auto& ps = scene.registry.ctx<component::global_state>();

		for (auto [e, pawn] : scene.registry.view<component::pawn>().each()) {
			if (pawn.client_idx != ps.local_client_idx)
				continue;

			auto& tick_input = scene.registry.get_or_emplace<component::pawn_tick_input>(e);
			tick_input.tick_input = _current_input;
		}

		game_run_tick(scene, _engine);

		update_camera(scene);

		auto connected = _engine.get_network().get_client().IsConnected();
		debug::draw_screen_text(scene, connected ? "Client: Connected" : "Client: Wait connect");
	}

	void client_ingame::update_camera(scene& scene) {
		auto& cam = scene.registry.ctx<component::camera>();
		auto& ps = scene.registry.ctx<component::global_state>();


		for (auto [e, p, tf] : scene.registry.view<component::pawn, component::transform>().each()) {
			if (p.client_idx != ps.local_client_idx)
				continue;

			cam.position = tf.position;
		}
	}

	game_input client_ingame::sample_current_game_input() {
		auto& input = _engine.get_input();

		game_input game_input;
		if (input.get_current_input_method() == input_method::keyboard_mouse) {
			game_input.up = input.is_key_down(ALLEGRO_KEY_W);
			game_input.down = input.is_key_down(ALLEGRO_KEY_S);
			game_input.left = input.is_key_down(ALLEGRO_KEY_A);
			game_input.right = input.is_key_down(ALLEGRO_KEY_D);
		}
		else {
			auto x_axis = input.get_joystick_axis(input_xbox_left_stick, 0);
			auto y_axis = input.get_joystick_axis(input_xbox_left_stick, 1);
			game_input.up = y_axis > 0.25f;
			game_input.down = y_axis < -0.25f;
			game_input.left = x_axis > 0.25f;
			game_input.right = x_axis < -0.25f;
		}
		return game_input;
	}

	void client_ingame::send_tick_input(size_t tick, game_input input) {
		auto msg = (network_messages::tick_input*)_engine.get_network().get_client().CreateMessage(TICK_INPUT_MESSAGE);
		msg->tick = tick;
		msg->input = input;

		_engine.get_network().get_client().SendMessage(0, msg);
	}
}