#pragma once

#include "math.h"

struct util_debug_line {
    math::vector2 from, to;
};

struct util_debug_text {
    math::vector2 position;
    std::string text;
};

struct util_global_data {
    std::vector<util_debug_line> debug_lines;
    std::vector<util_debug_text> debug_text;
};

void panic();

void init_utils(entt::registry& state);
void update_utils(entt::registry& state);

void debug_print(const char* lpFmt, ...);

void debug_draw_world_line(entt::registry& state, math::vector2 from, math::vector2 to);
void debug_draw_world_text(entt::registry& state, math::vector2 pos, std::string text);
