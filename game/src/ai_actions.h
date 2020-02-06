#pragma once

#include "game.h"


struct ai_agent;


enum class action_result {
    in_progress,
    failed,
    succeeded
};

enum class action_type : uint8_t {
    none,

    target_tree,
    kill_target,

    goto_target,

    target_deer,

    target_axe,

    pickup_target_item,

    target_human
};

constexpr const char* action_type_name[] {
    "none",

    "target_tree",
    "kill_target",

    "goto_target",

    "target_deer",

    "target_axe",

    "pickup_target_item",

    "target_human"
};

struct action {
    action_type type;
};

struct action_context {
    action action;
    ai_agent& agent;
    entt::entity agent_entity;
};

action_result target_tree(entt::registry& state, action_context& ctx);
action_result kill_target(entt::registry& state, action_context& ctx);

action_result goto_target(entt::registry& state, action_context& ctx);

action_result target_deer(entt::registry& state, action_context& ctx);

action_result target_axe(entt::registry& state, action_context& ctx);

action_result pickup_target_item(entt::registry& state, action_context& ctx);

action_result target_human(entt::registry& state, action_context& ctx);