#pragma once

#include "ai.h"
#include "game.h"

enum class action_result {
    in_progress,
    failed,
    succeeded
};

enum class action_type : uint8_t {
    none,
    target_tree,
    goto_target,
    kill_tree
};

struct action {
    action_type type;
};

struct action_context {
    action action;
    ai_agent& agent;
    position& agent_position;
};

action_result target_tree(entt::registry& state, action_context& ctx);
action_result goto_target(entt::registry& state, action_context& ctx);
action_result kill_tree(entt::registry& state, action_context& ctx);