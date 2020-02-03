#include "pch.h"
#include "ai_actions.h"
#include "utils.h"
#include "renderer.h"

action_result target_tree(entt::registry& state, action_context& ctx) {
    auto trees = state.view<tree>();
    if (trees.empty())
    {
        debug_print("no trees found");
        return action_result::failed;
    }

    ctx.agent.target = trees[rand() % trees.size()];
    return action_result::succeeded;
}

action_result goto_target(entt::registry& state, action_context& ctx) {
    if (!state.valid(ctx.agent.target))
    {
        debug_print("goto target lost");
        return action_result::failed;
    }

    auto target_pos = state.get<position>(ctx.agent.target);
    auto target_offset = (target_pos - ctx.agent_position);
    auto target_dist = target_offset.magnitude();
    if (target_dist < 0.1f)
        return action_result::succeeded;

    ctx.agent_position += (target_offset / target_dist) * std::min(6.f, target_dist);
    return action_result::in_progress;
}

action_result kill_tree(entt::registry& state, action_context& ctx) {
    if (!state.valid(ctx.agent.target) || !state.has<tree>(ctx.agent.target))
    {
        debug_print("kill target lost");
        return action_result::failed;
    }

    auto tree_stump = al_load_bitmap("data/tree_stump.png");
    state.replace<sprite>(ctx.agent.target, tree_stump);
    state.remove<tree>(ctx.agent.target);

    return action_result::succeeded;
}