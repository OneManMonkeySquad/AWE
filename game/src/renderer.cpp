
#include "pch.h"
#include "renderer.h"
#include <ppl.h>
#include "math.h"

position isoTo2D(position p) {
    position result;
    result.x = (2 * p.y + p.x) / 2;
    result.y = (2 * p.y - p.x) / 2;
    return result;
}

position twoDToIso(position p) {
    position result;
    result.x = p.x - p.y;
    result.y = (p.x + p.y) / 2;
    return result;
}

void create_tilemap(renderer_state& state, const engine_state& engine) {
    state.tiles = al_load_bitmap("data/tile.png");

    for (int y = 0; y < 100; y++) {
        for (int x = 0; x < 100; x++) {
            state.tile_map[x + y * 100] = rand() % 4;
        }
    }
}

void draw_tilemap(const renderer_state& state, const engine_state& engine) {
    al_hold_bitmap_drawing(1);

    int x, y;
    for (y = 0; y < 100; y++) {
        for (x = 0; x < 100; x++) {
            int i = state.tile_map[x + y * 100];
            float u = 1;
            float v = 1;

            auto isoPos = twoDToIso({ x * 16.f, y * 16.f });

            al_draw_scaled_bitmap(state.tiles, 
                u, v, 64, 32, 
                isoPos.x, isoPos.y,
                64, 32, 
                0);
        }
    }

    al_hold_bitmap_drawing(0);
}


renderer_state* create_renderer(const engine_state& engine) {
    auto new_renderer = new renderer_state();

    new_renderer->ttf_font = al_load_ttf_font("data/TIMES.TTF", 16, 0);

    new_renderer->main_camera.position = { 0, 0 };
    new_renderer->main_camera.rotate = 0;

    create_tilemap(*new_renderer, engine);

    return new_renderer;
}

entt::registry interpolate_for_rendering(const entt::registry& current_state, const entt::registry& previous_state, float a) {
    entt::registry interpolated_state;

    auto foo = current_state.view<const position, const sprite>();
    for (auto e : foo) {
        auto curr_pos = foo.get<const position>(e);

        auto old_pos = previous_state.try_get<const position>(e);
        auto interp_pos = math::lerp(old_pos != nullptr ? *old_pos : curr_pos, curr_pos, a);

        auto sp = foo.get<const sprite>(e);

        auto interp_e = interpolated_state.create();
        interpolated_state.assign<position>(interp_e, interp_pos);
        interpolated_state.assign<sprite>(interp_e, sp);
    }

    return interpolated_state;
}

static std::chrono::time_point<std::chrono::high_resolution_clock> old_time;

void render(const renderer_state& state, const engine_state& engine, const entt::registry& game_state) {
    using clock = std::chrono::high_resolution_clock;

    al_set_target_backbuffer(engine.display);

    al_clear_to_color(al_map_rgb(30, 30, 30));

    


    float w, h;
    w = al_get_display_width(engine.display);
    h = al_get_display_height(engine.display);


    ALLEGRO_TRANSFORM transform;
    al_identity_transform(&transform);
    al_translate_transform(&transform, -state.main_camera.position.x, -state.main_camera.position.y);
    al_rotate_transform(&transform, state.main_camera.rotate);
    al_scale_transform(&transform, state.main_camera.zoom, state.main_camera.zoom);
    al_translate_transform(&transform, w * 0.5, h * 0.5);
    al_use_transform(&transform);


   


   

    draw_tilemap(state, engine);


    al_hold_bitmap_drawing(1);

   
    
    
    std::vector<std::pair<float, entt::entity>> foo;

    auto rendables = game_state.view<const position, const sprite>();

    for (auto e : rendables) {
        auto& pos = rendables.get<const position>(e);
        auto& sp = rendables.get<const sprite>(e);

        foo.emplace_back(pos.y, e);
    }

    //Concurrency::parallel_radixsort(foo.begin(), foo.end(), [](auto pair) { return pair.first; });
    std::sort(foo.begin(), foo.end(), [](auto& lp, auto& rp) {
        return lp.first < rp.first;
    });

    for (auto p : foo) {
        auto pos = rendables.get<const position>(p.second);
        auto sp = rendables.get<const sprite>(p.second);

        auto isoPos = twoDToIso(pos);
        al_draw_bitmap(sp.bitmap, isoPos.x, isoPos.y, 0);
    }
    
    al_hold_bitmap_drawing(0);


    al_identity_transform(&transform);
    al_use_transform(&transform);



    using ms = std::chrono::duration<float, std::milli>;

    auto new_time = clock::now();
    auto delta = std::chrono::duration_cast<ms>(new_time - old_time).count();
    auto fps = static_cast<long>((1 / delta) * 1000);
    old_time = new_time;
    al_draw_textf(state.ttf_font, al_map_rgb(255, 255, 0), 16, 16, 0, "FPS: %d", fps);




    al_flip_display();
}
