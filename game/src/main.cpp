
#include "pch.h"
#include "meta.h"
#include "engine.h"
#include "game.h"
#include "renderer.h"
#include "utils.h"
#include "input.h"
#include "defer.h"
#include "editor.h"
#include "jobs.h"

using namespace entt::literals;



void update_camera(entt::registry& state, input& input, float delta_time_ms) {
	auto& cam = state.ctx<camera>();

	const auto wheel = input.get_mouse_wheel();
	const auto w = input.is_key_down(ALLEGRO_KEY_W);
	const auto s = input.is_key_down(ALLEGRO_KEY_S);
	const auto a = input.is_key_down(ALLEGRO_KEY_A);
	const auto d = input.is_key_down(ALLEGRO_KEY_D);

	cam.zoom = std::clamp(cam.zoom + wheel * delta_time_ms * 0.05f, 0.4f, 1.8f);
	cam.position.x += (d - a) * delta_time_ms * cam.zoom * 0.5f;
	cam.position.y += (s - w) * delta_time_ms * cam.zoom * 0.5f;
}




void read_file_and_print_size(void*) {
	auto content = jobs::read_file("D:/test.json");
	print("{}", content.size());
}

void DoFrames(void*) {
	while (true) {
		std::array decls{
			jobs::job_decl{&read_file_and_print_size, nullptr, "read_file_and_print_size"}
		};

		jobs::counter cnt;
		jobs::run_all(decls, &cnt);
	}
}

int main(int argc, char** argv) {
	jobs::context ctx;

	jobs::counter cnt;
	jobs::run_all(std::array{ jobs::job_decl{&DoFrames, nullptr} }, &cnt);

	ctx.run_blocking();

	print("TEST DONE");
	return 0;

	using namespace std::chrono;

	using clock = high_resolution_clock;
	using ms = duration<float, std::milli>;

	constexpr nanoseconds game_timestep = 16ms;
	constexpr nanoseconds max_single_timestep = 10ms;

	init_meta();

	engine engine{ "data/" };
	auto& renderer = engine.get_renderer();
	auto& input = engine.get_input();

	auto game_lag = 0ns;
	auto time_start = clock::now();

	auto game_state = game_create(engine);
	debug::init_utils(game_state);

	auto previous_game_state = clone_for_rendering(game_state); // Make sure the previous_state contains valid data

	auto quit = false;
	auto draw_debugger = true;
	while (!quit) {
		const auto delta_time = std::min(clock::now() - time_start, max_single_timestep);
		time_start = clock::now();

		game_lag += duration_cast<nanoseconds>(delta_time);


		quit |= engine.run_frame();

		for (const auto evt : input.events) {
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



		previous_game_state = clone_for_rendering(game_state);


		game_create_commands_from_input(game_state, input);
		while (game_lag >= game_timestep) {
			game_lag -= game_timestep;

			debug::update_utils(game_state);
			game_run_tick(game_state, engine);
		}

		{
			const auto delta_time_ms = duration_cast<ms>(delta_time).count();
			update_camera(game_state, input, delta_time_ms);
		}
		if (draw_debugger) {
			draw_debugger(game_state);
		}
		{
			auto a = static_cast<float>(game_lag.count() / game_timestep.count());
			auto rendered_state = interpolate_for_rendering(game_state, previous_game_state, a);

			auto& cam = game_state.ctx<camera>();
			renderer.render(cam, rendered_state);
		}

		yield();
	}

	print("=== Shutdown ===");
	return 0;
}