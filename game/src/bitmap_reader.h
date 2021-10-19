
#pragma once

#include "expected.h"

struct ALLEGRO_BITMAP;

namespace bitmap_reader {
	expected<ALLEGRO_BITMAP*> load_bitmap(std::string_view path);
}