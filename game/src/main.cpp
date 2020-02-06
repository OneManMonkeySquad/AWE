
#include "pch.h"
#include "engine.h"
#include "game.h"
#include "renderer.h"
#include "utils.h"
#include "input.h"


using namespace std;


int main(int argc, char** argv) {
    using clock = chrono::high_resolution_clock;

    constexpr chrono::nanoseconds timestep = 16ms;

    auto engine = create_engine();

    chrono::nanoseconds lag = 0ns;
    auto time_start = clock::now();

    entt::registry current_state = create_game();
    init_utils(current_state);

    entt::registry previous_state;

    bool quit_game = false;
    while (!quit_game) {
        const auto delta_time = clock::now() - time_start;

        using ms = chrono::duration<float, std::milli>;
        const auto delta_time_ms = chrono::duration_cast<ms>(delta_time).count();

        time_start = clock::now();
        lag += chrono::duration_cast<chrono::nanoseconds>(delta_time);

        quit_game = handle_input(*engine, current_state);

        // update game logic as lag permits
        while (lag >= timestep) {
            lag -= timestep;

            previous_state = current_state.clone();
            update(current_state); // update at a fixed rate each time
        }

        update_camera(current_state, delta_time_ms);

        // calculate how close or far we are from the next timestep
        auto a = (float)lag.count() / timestep.count();
        auto rendered_state = interpolate_for_rendering(current_state, previous_state, a);

        auto& cam = current_state.ctx<camera>();
        render(engine, cam, rendered_state);
    }

    destroy_engine(engine);

    return 0;
}