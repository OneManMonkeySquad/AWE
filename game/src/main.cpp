
#include "pch.h"
#include "meta.h"
#include "game.h"
#include "editor.h"
#include "game_network_adapter.h"
#include "fabrik/allegro_renderer.h"
#include "fabrik/allegro_input.h"
#include "fabrik/null_renderer.h"
#include "fabrik/null_input.h"

using namespace entt::literals;
using namespace std::literals::chrono_literals;










void update_camera(scene& scene, input& input, float delta_time_ms) {
	auto& cam = scene.registry.ctx<camera>();

	const auto wheel = input.get_mouse_wheel();

	cam.zoom = std::clamp(cam.zoom + wheel * delta_time_ms * 0.05f, 0.4f, 1.8f);

	for (auto [e, tf] : scene.registry.view<pawn, transform>().each()) {
		cam.position = tf.position;
	}
}









namespace app_states {
	class client_download_scene;

	class server_host : public app_state {
	public:
		server_host(engine& engine)
			: app_state(engine) {}

		void enter() {
			_engine.get_network().start_server(std::make_unique<game_network_adapter>(_engine), 60000);

			_engine.get_event_hub().add_listener_instance<events::client_connected>(this);
			_engine.get_event_hub().add_listener_instance<events::client_disconnected>(this);
		}
		void exit() {}
		void begin_frame() {}

		void on_event(const events::client_connected& evt) {
			auto& scene = _engine.get_scene_manager().get_first_scene();

			add_player(_engine, scene);
			send_initial_game_state(_engine, scene, evt.client_idx);
		}

		void on_event(const events::client_disconnected& evt) {}


	};

	class client_disconnected : public app_state {
	public:
		client_disconnected(engine& engine, std::string reason)
			: app_state(engine) {}

		void enter() {}
		void exit() {}
		void begin_frame() {}
	};

	class client_connect : public app_state {
	public:
		client_connect(engine& engine)
			: app_state(engine) {}

		void enter() {
			_engine.get_network().start_client(std::make_unique<game_network_adapter>(_engine));
			_engine.get_network().connect("127.0.0.1", 60000);
		}
		void exit() {}
		void begin_frame() {
			if (_engine.get_network().get_client().IsConnected()) {
				_engine.get_app_state_manager().switch_to<client_download_scene>();
			}
		}
	};

	class client_download_scene : public app_state {
	public:
		client_download_scene(engine& engine)
			: app_state(engine) {}

		void enter() {}
		void exit() {}
		void begin_frame() {
			if (!_engine.get_network().get_client().IsConnected()) {
				_engine.get_app_state_manager().switch_to<client_disconnected>("Connection Lost");
			}
		}
	};
}


constexpr std::chrono::nanoseconds game_timestep = 16ms;
constexpr std::chrono::nanoseconds max_single_timestep = 10ms;



void run_game(engine& engine) {
	using namespace std::chrono;

	auto& renderer = engine.get_renderer();
	auto& input = engine.get_input();
	auto& network = engine.get_network();

	auto game_lag = 0ns;
	auto time_start = high_resolution_clock::now();

	auto& main_scene = game_create(engine);
	debug::init_utils(main_scene);

	if (network.is_server()) {
		game_add_server_stuff(main_scene, engine);
	}

	auto previous_rendable_registry = renderer.clone_for_rendering(main_scene); // Make sure the previous_state contains valid data

	std::string initial_game_state_buffer;

	auto quit = false;
	auto draw_debugger = false;
	while (!quit) {
		const auto delta_time = std::min(high_resolution_clock::now() - time_start, max_single_timestep);
		time_start = high_resolution_clock::now();
		game_lag += duration_cast<nanoseconds>(delta_time);

		quit |= engine.begin_frame();

		for (const auto& evt : input.events) {
			switch (evt.type) {
			case input_event_type::key_down:
				if (evt.key == ALLEGRO_KEY_ESCAPE) {
					quit = true;
				}
				if (evt.key == ALLEGRO_KEY_F1) {
					draw_debugger = !draw_debugger;
				}
			}
		}


		if (network.is_client()) {
			auto connected = network.get_client().IsConnected();
			debug::draw_screen_text(main_scene, connected ? "Client: Connected" : "Client: Wait connect");

			for (auto evt : network.client_messages) {
				switch (evt->GetType()) {
				case (int)INITIAL_GAME_STATE_MESSAGE:
					initial_game_state_buffer += ((network_messages::initial_game_state*)evt)->data;
					break;
				case (int)INITIAL_GAME_STATE_END_MESSAGE:
					auto hash = std::hash<std::string>()(initial_game_state_buffer);
					if (hash == ((network_messages::initial_game_state_end*)evt)->data_hash) {
						on_initial_game_state(engine, main_scene, initial_game_state_buffer);
					}
					else {
						engine.get_app_state_manager().switch_to<app_states::client_disconnected>("Initial Game State Corrupt");
					}
					initial_game_state_buffer.clear();
					break;
				}
			}
		}

		previous_rendable_registry = renderer.clone_for_rendering(main_scene);

		game_create_commands_from_input(main_scene, input);
		while (game_lag >= game_timestep) {
			game_lag -= game_timestep;

			debug::update_utils(main_scene);
			game_run_tick(main_scene, engine);
		}

		{
			const auto delta_time_ms = duration_cast<duration<float, std::milli>>(delta_time).count();
			update_camera(main_scene, input, delta_time_ms);
		}
		if (draw_debugger) {
			::draw_debugger(main_scene);
		}
		{
			auto a = static_cast<float>(game_lag.count() / game_timestep.count());
			auto interpolated_rendable_registry = renderer.interpolate_for_rendering(main_scene.registry, previous_rendable_registry, a);

			auto& cam = main_scene.registry.ctx<camera>();
			renderer.render(cam, interpolated_rendable_registry);
		}

		yield();
	}
}

void run_client(void*) {
	engine engine{ "data/",
		std::make_unique<allegro_renderer>("AWE"),
		std::make_unique<allegro_input>(),
		std::make_unique<yojimbo_network>()
	};

	engine.get_app_state_manager().switch_to<app_states::client_connect>();

	run_game(engine);
}

void run_server(void*) {
	engine engine{ "data/",
		std::make_unique<null_renderer>(),
		std::make_unique<null_input>(),
		std::make_unique<yojimbo_network>()
	};

	engine.get_app_state_manager().switch_to<app_states::server_host>();

	run_game(engine);
}

int main(int argc, char** argv) {
	init_meta();

	if (!awe_init())
		panic("AWE init failed");
	defer{
		awe_shutdown();
	};

	jobs::context ctx;

	jobs::counter cnt;
	jobs::run_all(std::array{
		jobs::job_decl{&run_server, nullptr, "run_server"},
		jobs::job_decl{&run_client, nullptr, "run_client"}
				  }, & cnt);

	ctx.run_blocking();

	return 0;
}