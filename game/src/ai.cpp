
#include "pch.h"
#include "ai.h"
#include "game.h"
#include "utils.h"
#include "renderer.h"
#include "ai_actions.h"
#include <string>


plan create_plan();


template<auto FuncUpdateT>
void add_action(ai_global_data& global_data) {
    global_data.actions.push_back({ entt::connect_arg<FuncUpdateT> });
}


void init_ai(entt::registry& state) {
    auto& global_data = state.ctx_or_set<ai_global_data>();
    global_data.ai_agent_created.connect(state, entt::collector.group<ai_agent>());

    add_action<&target_tree>(global_data);
    add_action<&kill_target>(global_data);

    add_action<&goto_target>(global_data);

    add_action<&target_deer>(global_data);

    add_action<&target_axe>(global_data);

    add_action<&pickup_target_item>(global_data);

    add_action<&target_human>(global_data);
}


void update_ai(entt::registry& state) {
    ai_global_data& global_data = state.ctx<ai_global_data>();

    global_data.ai_agent_created.each([&](entt::entity e) {
        state.assign<ai_agent_system_data>(e);
    });

    auto ai_agents = state.view<ai_agent, ai_agent_system_data, position>();
    for (auto agent_entity : ai_agents) {
        auto& agent = ai_agents.get<ai_agent>(agent_entity);
        auto& agent_system_data = ai_agents.get<ai_agent_system_data>(agent_entity);

        auto& current_action = agent_system_data.current_action;
        if (current_action.type != action_type::none) {
            assert(static_cast<size_t>(current_action.type) <= global_data.actions.size()); // <= weil erster action_type::none ist

            debug_draw_world_text(state, state.get<position>(agent_entity) + math::vector2{ 40, 40 }, action_type_name[static_cast<unsigned int>(current_action.type)]);

            const auto& action = global_data.actions[static_cast<unsigned int>(current_action.type) - 1]; // -1 weil erster action_type::none ist

            action_context ctx{
                current_action.type,
                agent,
                agent_entity
            };
            const auto result = action(state, ctx);
            switch (result) {
            case action_result::in_progress:
                continue;

            case action_result::failed:
                current_action.type = action_type::none;
                agent_system_data.current_plan.clear();

            case action_result::succeeded:
                break;
            }
        }

        // Nächste Aktion
        if (!agent_system_data.current_plan.empty()) {
            agent_system_data.current_action = agent_system_data.current_plan.back();
            agent_system_data.current_plan.pop_back();
        }
        else {
            agent_system_data.current_plan = create_plan();
        }
    }
}


plan create_plan() {
    plan p;

    p.push_back({ action_type::kill_target });
    p.push_back({ action_type::goto_target });
    p.push_back({ action_type::target_human });

    p.push_back({ action_type::kill_target });
    p.push_back({ action_type::goto_target });
    p.push_back({ action_type::target_deer });

    p.push_back({ action_type::kill_target });
    p.push_back({ action_type::goto_target });
    p.push_back({ action_type::target_tree });

    p.push_back({ action_type::pickup_target_item });
    p.push_back({ action_type::goto_target });
    p.push_back({ action_type::target_axe });

    return p;
}