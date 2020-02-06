#include "pch.h"
#include "ai_actions.h"
#include "ai.h"
#include "utils.h"
#include "renderer.h"


action_result target_tree(entt::registry& state, action_context& ctx) {
    auto trees = state.view<tree>();
    if (trees.empty())
        return action_result::failed;

    ctx.agent.target = trees[rand() % trees.size()];
    return action_result::succeeded;
}


action_result kill_target(entt::registry& state, action_context& ctx) {
    if (!state.valid(ctx.agent.target) /*|| !state.has<tree>(ctx.agent.target)*/)
        return action_result::failed;

    kill(state, ctx.agent.target);
    return action_result::succeeded;
}


action_result goto_target(entt::registry& state, action_context& ctx) {
    if (!state.valid(ctx.agent.target))
        return action_result::failed;

    auto* target_pos = state.try_get<position>(ctx.agent.target);
    if (target_pos == nullptr)
        return action_result::failed;

    auto& pos = state.get<position>(ctx.agent_entity);
    debug_draw_world_line(state, pos, *target_pos);
    auto target_offset = (*target_pos - pos);
    auto target_dist = target_offset.magnitude();
    if (target_dist <= 1.f)
        return action_result::succeeded;

    pos += target_dist > 2.f ? (target_offset / target_dist) * 2 : target_offset;
    return action_result::in_progress;
}


action_result target_deer(entt::registry& state, action_context& ctx) {
    auto deers = state.view<deer>();
    if (deers.empty())
        return action_result::failed;

    ctx.agent.target = deers[rand() % deers.size()];
    return action_result::succeeded;
}


action_result target_axe(entt::registry& state, action_context& ctx) {
    ctx.agent.target = entt::null;

    std::vector<entt::entity> axes;

    auto items = state.view<item, position>(); // position weil wir nur Items auf dem Boden wollen
    for (auto e : items) {
        auto it = items.get<item>(e);
        if (it.type == item_type::axe) {
            axes.push_back(e);
        }
    }

    if (axes.empty())
        return action_result::failed;

    ctx.agent.target = axes[rand() % axes.size()];
    return action_result::succeeded;
}


action_result pickup_target_item(entt::registry& state, action_context& ctx) {
    if (!state.valid(ctx.agent.target) || !state.has<item>(ctx.agent.target))
        return action_result::failed;

    auto& inv = state.get<inventory>(ctx.agent_entity);
    if (!inventory_add_item(state, inv, ctx.agent.target))
        return action_result::failed;

    return action_result::succeeded;
}


action_result target_human(entt::registry& state, action_context& ctx) {
    auto ais = state.view<ai_agent>();
    if (ais.empty())
        return action_result::failed;

    ctx.agent.target = ais[rand() % ais.size()];
    return action_result::succeeded;
}