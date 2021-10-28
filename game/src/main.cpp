
#include "pch.h"
#include "meta.h"
#include "game.h"
#include "editor.h"
#include "game_network_adapter.h"
#include "app_states.h"
#include "fabrik/allegro_resource_manager.h"
#include "fabrik/allegro_renderer.h"
#include "fabrik/allegro_input.h"
#include "fabrik/null_renderer.h"
#include "fabrik/null_input.h"
#include "fabrik/null_resource_manager.h"

using namespace entt::literals;
using namespace std::literals::chrono_literals;

constexpr std::chrono::nanoseconds game_timestep = 16ms;
constexpr std::chrono::nanoseconds max_single_timestep = 10ms;



auto quit = false;
void run_game(engine& engine) {
	using namespace std::chrono;

	auto& renderer = engine.get_renderer();
	auto& input = engine.get_input();
	auto& network = engine.get_network();

	auto game_lag = 0ns;
	auto time_start = high_resolution_clock::now();

	auto& main_scene = game_create(engine);
	debug::init_utils(main_scene);

	auto previous_rendable_registry = renderer.clone_for_rendering(main_scene); // Make sure the previous_state contains valid data

	auto draw_debugger = false;
	while (!quit) {
		const auto delta_time = std::min(high_resolution_clock::now() - time_start, max_single_timestep);
		time_start = high_resolution_clock::now();
		game_lag += duration_cast<nanoseconds>(delta_time);

		engine.begin_frame();

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

		while (game_lag >= game_timestep) {
			game_lag -= game_timestep;

			previous_rendable_registry = renderer.clone_for_rendering(main_scene);

			debug::update_utils(main_scene);
			engine.tick();
		}

		if (draw_debugger) {
			debugger_draw(main_scene);
		}
		{
			const auto a = static_cast<float>(game_lag.count() / game_timestep.count());
			fabrik_assert(a >= 0 && a <= 1);

			const auto interpolated_rendable_registry = renderer.interpolate_for_rendering(main_scene.registry, previous_rendable_registry, a);

			const auto& cam = main_scene.registry.ctx<component::camera>();
			renderer.render(cam, interpolated_rendable_registry);
		}

		yield();
	}
}

void run_client(void*) {
	engine engine{
		std::make_unique<allegro_resource_manager>("data/"),
		std::make_unique<allegro_renderer>("AWE"),
		std::make_unique<allegro_input>(),
		std::make_unique<yojimbo_network>()
	};

	engine.get_app_state_manager().switch_to<app_states::client_connect>();

	run_game(engine);
}

void run_server(void*) {
	engine engine{
		std::make_unique<null_resource_manager>(),
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
	defer{ awe_shutdown(); };

	jobs::context ctx;

	cxxopts::Options options("AWE", "Description");
	options.add_options()
		("s,server", "Start as server", cxxopts::value<bool>()->default_value("false"))
		("c,client", "Start as client", cxxopts::value<bool>()->default_value("false"))
		;
	auto result = options.parse(argc, argv);

	std::vector<jobs::job_decl> jobs;
	if (result["server"].as<bool>()) {
		create_console_window();
		jobs.push_back(jobs::job_decl{ &run_server, nullptr, "run_server" });
	}
	else if (result["client"].as<bool>()) {
		jobs.push_back(jobs::job_decl{ &run_client, nullptr, "run_client" });
	}
	else {
		jobs.push_back(jobs::job_decl{ &run_server, nullptr, "run_server" });
		jobs.push_back(jobs::job_decl{ &run_client, nullptr, "run_client" });
	}

	jobs::counter cnt;
	jobs::run_all(jobs, &cnt);

	ctx.run_blocking();

	return 0;
}