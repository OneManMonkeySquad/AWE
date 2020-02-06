#pragma once

namespace math {
struct vector2;
}

using position = math::vector2;

struct velocity {
    float dx;
    float dy;
};

enum class item_type {
    axe
};

struct item {
    item_type type;
};

struct inventory {
    std::array<entt::entity, 3> items = { entt::null ,entt::null ,entt::null };
};

using tree = entt::tag<"tree"_hs>;
using deer = entt::tag<"deer"_hs>;
using wood = entt::tag<"wood"_hs>;

entt::registry create_game();

void update(entt::registry& state);
void update_camera(entt::registry& state, float delta_time_ms);

void kill(entt::registry& state, entt::entity target);

bool inventory_add_item(entt::registry& state, inventory& inventory, entt::entity new_item);
bool inventory_has_item_of_type(entt::registry& state, const inventory& inventory, item_type type);