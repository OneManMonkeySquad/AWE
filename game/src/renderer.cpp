
#include "pch.h"
#include "renderer.h"
#include "math.h"
#include "engine.h"

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

void create_tilemap(renderer_state& state) {
    state.tiles = al_load_bitmap("data/tile.png");

    for (int y = 0; y < 100; y++) {
        for (int x = 0; x < 100; x++) {
            state.tile_map[x + y * 100] = rand() % 4;
        }
    }
}

void draw_tilemap(const renderer_state* renderer) {
    al_hold_bitmap_drawing(1);

    int x, y;
    for (y = 0; y < 100; y++) {
        for (x = 0; x < 100; x++) {
            int i = renderer->tile_map[x + y * 100];
            float u = 1;
            float v = 1;

            auto isoPos = twoDToIso({ x * 16.f, y * 16.f });

            al_draw_scaled_bitmap(renderer->tiles,
                u, v, 64, 32, 
                isoPos.x, isoPos.y,
                64, 32, 
                0);
        }
    }

    al_hold_bitmap_drawing(0);
}


renderer_state* create_renderer() {
    auto new_renderer = new renderer_state();

    al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_FRAMELESS);
    new_renderer->display = al_create_display(1280, 1024);
    al_set_window_title(new_renderer->display, "Awe");

    new_renderer->ttf_font = al_load_ttf_font("data/TIMES.TTF", 16, 0);

    create_tilemap(*new_renderer);

    return new_renderer;
}

void destroy_renderer(renderer_state* state) {
    al_destroy_display(state->display);
}

entt::registry interpolate_for_rendering(const entt::registry& current_state, const entt::registry& previous_state, float a) {
    entt::registry interpolated_state;

    auto foo = current_state.view<const position, const sprite>();
    for (auto e : foo) {
        auto curr_pos = foo.get<const position>(e);

        position old_pos;
        if (previous_state.valid(e)) { // Entity könnte erst diesen Frame existieren
            old_pos = previous_state.get<const position>(e);
        }
        else {
            old_pos = curr_pos;
        }

        auto interp_pos = math::lerp(old_pos, curr_pos, a);

        auto sp = foo.get<const sprite>(e);

        auto interp_e = interpolated_state.create();
        interpolated_state.assign<position>(interp_e, interp_pos);
        interpolated_state.assign<sprite>(interp_e, sp);
    }

    return interpolated_state;
}

static std::chrono::time_point<std::chrono::high_resolution_clock> old_time;

void render(const engine_state* engine, const camera cam, const entt::registry& game_state) {
    using clock = std::chrono::high_resolution_clock;

    auto renderer = engine->renderer;

    al_set_target_backbuffer(renderer->display);

    al_clear_to_color(al_map_rgb(30, 30, 30));



    float w, h;
    w = al_get_display_width(renderer->display);
    h = al_get_display_height(renderer->display);


    ALLEGRO_TRANSFORM transform;
    al_identity_transform(&transform);
    al_translate_transform(&transform, -cam.position.x, -cam.position.y);
    al_rotate_transform(&transform, cam.rotate);
    al_scale_transform(&transform, cam.zoom, cam.zoom);
    al_translate_transform(&transform, w * 0.5, h * 0.5);

    al_use_transform(&transform);




    draw_tilemap(engine->renderer);


    

    std::vector<std::pair<float, entt::entity>> depth_rendable_pairs;

    auto rendables = game_state.view<const position, const sprite>();

    for (auto e : rendables) {
        auto& pos = rendables.get<const position>(e);
        auto& sp = rendables.get<const sprite>(e);

        depth_rendable_pairs.emplace_back(pos.y, e);
    }

    std::sort(depth_rendable_pairs.begin(), depth_rendable_pairs.end(), [](auto& lp, auto& rp) {
        return lp.first < rp.first;
    });

    al_hold_bitmap_drawing(true);
    for (auto p : depth_rendable_pairs) {
        auto pos = rendables.get<const position>(p.second);
        auto sp = rendables.get<const sprite>(p.second);

        auto isoPos = twoDToIso(pos);
        al_draw_bitmap(sp.bitmap, isoPos.x, isoPos.y, 0);
    }
    al_hold_bitmap_drawing(false);


    al_identity_transform(&transform);
    al_use_transform(&transform);



    using ms = std::chrono::duration<float, std::milli>;

    auto new_time = clock::now();
    auto delta = std::chrono::duration_cast<ms>(new_time - old_time).count();
    auto fps = static_cast<long>((1 / delta) * 1000);
    old_time = new_time;
    al_draw_textf(renderer->ttf_font, al_map_rgb(255, 255, 0), 16, 16, 0, "FPS: %d", fps);




    al_flip_display();
}

