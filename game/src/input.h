#pragma once

struct engine_state;

struct input {
    bool w, a, s, d;
    float wheel;
};

bool handle_input(engine_state& engine, entt::registry& state);