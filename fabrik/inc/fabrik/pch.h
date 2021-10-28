
#pragma once

#define NOMINMAX

#include <iostream>
#include <chrono>
#include <algorithm>
#include <filesystem>
#include <string>
#include <format>
#include <thread>
#include <type_traits>
#include <span>

#include "entt/entt.hpp"

#include "imgui/imgui.h"

#include <allegro5/allegro5.h>
#include "allegro5/allegro_font.h"
#include "allegro5/allegro_ttf.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_primitives.h"

#include "tracy/Tracy.hpp"

#include "yojimbo/yojimbo.h"

#include "cxxopts.hpp"

#include "cereal/cereal.hpp"
#include "cereal/archives/json.hpp"