#include "pch.h"
#include "utils.h"
#include <stdio.h>
#include <process.h>
#include <windows.h>

void panic() {
    __debugbreak();
    abort();
}

void debug_print(const char* lpFmt, ...)
{
    char dbgmsg[200];
    va_list arglist;

    va_start(arglist, lpFmt);
    auto written = _vsnprintf_s(dbgmsg, sizeof(dbgmsg), lpFmt, arglist);
    va_end(arglist);
    written = std::min(written, (int)std::size(dbgmsg) - 2);
    dbgmsg[written] = '\n';
    dbgmsg[written + 1] = '\0';

    OutputDebugString(dbgmsg);
}


void init_utils(entt::registry& state) {
    debug_print("init_utils");
    state.set<util_global_data>();
}


void update_utils(entt::registry& state) {
    auto& global_data = state.ctx<util_global_data>();
    global_data.debug_lines.clear();
    global_data.debug_text.clear();
}


void debug_draw_world_line(entt::registry& state, math::vector2 from, math::vector2 to) {
    auto& global_data = state.ctx<util_global_data>();
    global_data.debug_lines.push_back({ from, to });
}


void debug_draw_world_text(entt::registry& state, math::vector2 pos, std::string text) {
    auto& global_data = state.ctx<util_global_data>();
    global_data.debug_text.push_back({ pos, text });
}