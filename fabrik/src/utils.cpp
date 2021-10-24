#include "pch.h"
#include "utils.h"
#include "scene.h"
#include <stdio.h>
#include <process.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

[[noreturn]]
void panic(error err) {
	MessageBoxA(0, err.what(), "Fatal Error", MB_OK);
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
	void set_current_thread_name(const char* name) {
		SetThreadDescription(GetCurrentThread(), to_wstring(name).c_str());
		tracy::SetThreadName(name);
	}

	void init_utils(scene& scene) {
		scene.registry.set<internal::util_global_data>();
	}


	void update_utils(scene& scene) {
		auto& global_data = scene.registry.ctx<internal::util_global_data>();
		global_data.world_lines.clear();
		global_data.world_text.clear();
		global_data.screen_text.clear();
		global_data.num_screen_lines = 0;
	}


	void draw_world_line(scene& scene, math::vector2 from, math::vector2 to, math::color color) {
		auto& global_data = scene.registry.ctx<internal::util_global_data>();
		global_data.world_lines.push_back({ from, to, color });
	}


	void draw_world_text(scene& scene, math::vector2 pos, std::string text, math::color color) {
		auto& global_data = scene.registry.ctx<internal::util_global_data>();
		global_data.world_text.push_back({ pos, color, text });
	}

	void draw_screen_text_at(scene& scene, math::vector2 pos, std::string text, math::color color) {
		auto& global_data = scene.registry.ctx<internal::util_global_data>();
		global_data.screen_text.push_back({ pos, color, text });
	}

	void draw_screen_text(scene& scene, std::string text, math::color color) {
		auto& global_data = scene.registry.ctx<internal::util_global_data>();

		math::vector2 pos{ 10, static_cast<float>(10 + 10 * global_data.num_screen_lines++) };
		global_data.screen_text.push_back({ pos, color, text });
	}
}