#pragma once

#include "math.h"
#include "error.h"

[[noreturn]]
void panic(error err);

void yield();

std::wstring to_wstring(std::string_view str);

void print(std::string_view str);
template <class... Ts>
void print(std::string_view fmt, const Ts&... args) {
	const auto str = std::format(fmt, args...);
	print(str.c_str());
}

namespace debug {
	namespace internal {
		struct debug_line {
			math::vector2 from, to;
			math::color color;
		};

		struct debug_text {
			math::vector2 position;
			math::color color;
			std::string text;
		};

		struct util_global_data {
			std::vector<debug_line> world_lines;
			std::vector<debug_text> world_text;
			std::vector<debug_text> screen_text;
		};
	}

	void init_utils(entt::registry& state);
	void update_utils(entt::registry& state);

	void draw_world_line(entt::registry& state, math::vector2 from, math::vector2 to, math::color color = math::color::white);
	void draw_world_text(entt::registry& state, math::vector2 pos, std::string text, math::color color = math::color::white);
	void draw_screen_text(entt::registry& state, math::vector2 pos, std::string text, math::color color = math::color::white);
}

