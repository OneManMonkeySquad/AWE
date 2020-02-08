#include "pch.h"
#include "ai_actions.h"
#include "ai.h"
#include "utils.h"
#include "renderer.h"


bool goto_target(entt::registry& state, action_context& ctx) {
    if (!state.valid(ctx.agent.target))
        return false;

    auto target_pos = state.try_get<position>(ctx.agent.target);
    if (target_pos == nullptr)
        return false;

    auto& pos = state.get<position>(ctx.agent_entity);

    auto diff = (*target_pos - pos);
    auto dist = diff.magnitude();

    const auto move_speed = 6;
    pos += dist > move_speed ? (diff / dist) * move_speed : diff;

    return dist < 1;
}


action_result hunt_deer(entt::registry& state, action_context& ctx) {
    auto target_invalid = !state.valid(ctx.agent.target) || !state.has<deer>(ctx.agent.target);
    if (target_invalid) {
        auto deers = state.view<deer>();
        if (deers.empty())
            return action_result::failed;

        ctx.agent.target = deers[rand() % deers.size()];
    }

    if (!goto_target(state, ctx))
        return action_result::in_progress;

    kill(state, ctx.agent.target);
    return action_result::succeeded;
}


action_result pickup_meat(entt::registry& state, action_context& ctx) {
    auto target_invalid = !state.valid(ctx.agent.target) || !state.has<item>(ctx.agent.target);
    if (target_invalid || state.get<item>(ctx.agent.target).type != item_type::meat) {
        auto items = state.view<item, position>();
        if (items.empty())
            return action_result::failed;

        std::vector<entt::entity> meats;
        for (auto item_entity : items) {
            auto it = items.get<item>(item_entity);
            if (it.type == item_type::meat) {
                meats.push_back(item_entity);
            }
        }
        if (meats.empty())
            return action_result::failed;

        ctx.agent.target = meats[rand() % meats.size()];
    }

    if (!goto_target(state, ctx))
        return action_result::in_progress;

    inventory_add_item(state, ctx.agent_entity, ctx.agent.target);
    return action_result::succeeded;
}


action_result eat_meat(entt::registry& state, action_context& ctx) {
    auto& inv = state.get<inventory>(ctx.agent_entity);
    
    if (!inventory_has_item_of_type(state, ctx.agent_entity, item_type::meat))
        return action_result::failed;
    
    auto item = inventory_remove_item_of_type(state, ctx.agent_entity, item_type::meat);
    if (item == entt::null)
        return action_result::failed;
    
    state.destroy(item);
    ctx.agent.hunger = 0;

    return action_result::succeeded;
}