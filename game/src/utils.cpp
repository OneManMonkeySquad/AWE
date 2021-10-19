#include "pch.h"
#include "utils.h"
#include <stdio.h>
#include <process.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

[[noreturn]]
void panic(error err) {
	print(std::format("ABORT {}", err.what()));
	std::terminate();
}

void yield() {
	Sleep(0);
}

std::wstring to_wstring(std::string_view str) {
	auto ws = std::make_unique<wchar_t[]>(str.size() + 1);
	mbstowcs_s(nullptr, ws.get(), str.size() + 1, str.data(), str.size());
	return { ws.get() };
}

void print(std::string_view str) {
	if (str.size() < 256 - 3) {
		char buff[256];
		memcpy(buff, str.data(), str.size());
		buff[str.size()] = '\n';
		buff[str.size() + 1] = '\0';
		OutputDebugStringA(buff);
	}
	else {
		const auto buff = std::format("{}\n", str);
		OutputDebugStringA(buff.c_str());
	}
}


namespace debug {
	void init_utils(entt::registry& state) {
		state.set<internal::util_global_data>();
	}


	void update_utils(entt::registry& state) {
		auto& global_data = state.ctx<internal::util_global_data>();
		global_data.world_lines.clear();
		global_data.world_text.clear();
		global_data.screen_text.clear();
	}


	void draw_world_line(entt::registry& state, math::vector2 from, math::vector2 to, math::color color) {
		auto& global_data = state.ctx<internal::util_global_data>();
		global_data.world_lines.push_back({ from, to, color });
	}


	void draw_world_text(entt::registry& state, math::vector2 pos, std::string text, math::color color) {
		auto& global_data = state.ctx<internal::util_global_data>();
		global_data.world_text.push_back({ pos, color, text });
	}

	void draw_screen_text(entt::registry& state, math::vector2 pos, std::string text, math::color color) {
		auto& global_data = state.ctx<internal::util_global_data>();
		global_data.screen_text.push_back({ pos, color, text });
	}
}