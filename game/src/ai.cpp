
#include "pch.h"
#include "ai.h"
#include "game.h"
#include "utils.h"
#include "renderer.h"
#include "ai_actions.h"

struct ai_global_data {
    entt::observer ai_agent_created;
    std::vector< entt::delegate< action_result(entt::registry&, action_context&) > > actions;
};

using plan = std::vector<action>;

struct ai_agent_system_data {
    plan current_plan;
    action current_action;
};

void init_ai(entt::registry& state) {
    auto& global_data = state.ctx_or_set<ai_global_data>();
    global_data.ai_agent_created.connect(state, entt::collector.group<ai_agent>());

    global_data.actions = {
        {},
        { entt::connect_arg<&target_tree> },
        { entt::connect_arg<&goto_target> },
        { entt::connect_arg<&kill_tree> }
    };
}

plan create_plan() {
    plan p;
    p.push_back({ action_type::kill_tree });
    p.push_back({ action_type::goto_target });
    p.push_back({ action_type::target_tree });
    return p;
}

void update_ai(entt::registry& state) {
    ai_global_data& global_data = state.ctx<ai_global_data>();

    global_data.ai_agent_created.each([&](entt::entity e) {
        state.assign<ai_agent_system_data>(e);
        debug_print("Assigned system data to %u", (uint32_t)e);
    });

    state.view<ai_agent, ai_agent_system_data, position>().each([&state, &global_data](ai_agent& agent, ai_agent_system_data& system_data, position& pos) {
        auto& current_action = system_data.current_action;
        if (current_action.type != action_type::none) {
            assert(static_cast<size_t>(current_action.type) < global_data.actions.size());

            const auto& action = global_data.actions[static_cast<unsigned int>(current_action.type)];

            action_context ctx{ 
                current_action.type,
                agent, 
                pos 
            };
            const auto result = action(state, ctx);
            switch (result) {
            case action_result::in_progress:
                return;

            case action_result::failed:
                debug_print("Action failed");
                current_action.type = action_type::none;
                system_data.current_plan.clear();

            case action_result::succeeded:
                break;
            }
        }

        // Nächste Aktion
        if (!system_data.current_plan.empty()) {
            system_data.current_action = system_data.current_plan.back();
            system_data.current_plan.pop_back();
        }
        else {
            system_data.current_plan = create_plan();
        }
    });
}