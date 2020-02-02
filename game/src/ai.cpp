
#include "pch.h"
#include "ai.h"
#include "game.h"
#include "utils.h"
#include "renderer.h"

enum class action_type : uint8_t {
    none,
    target_tree,
    goto_target,
    kill_tree
};

struct action {
    action_type type;
};

struct ai_global_data {
    entt::observer ai_agent_created;
};

using plan = std::vector<action>;

struct ai_agent_system_data {
    plan current_plan;
    action current_action;
};

void init_ai(entt::registry& state) {
    debug_print("init_ai");

    auto& global_data = state.ctx_or_set<ai_global_data>();
    global_data.ai_agent_created.connect(state, entt::collector.group<ai_agent>());
}

plan create_plan() {
    plan p;
    p.push_back({ action_type::kill_tree });
    p.push_back({ action_type::goto_target });
    p.push_back({ action_type::target_tree });
    return p;
}

enum class action_result {
    in_progress,
    failed,
    succeeded
};

action_result execute_action(entt::registry& state, ai_agent& agent, position& agent_position, action action) {
    switch (action.type) {
    case action_type::target_tree: {
        auto trees = state.view<tree>();
        if (trees.empty())
        {
            debug_print("no trees found");
            return action_result::failed;
        }

        agent.target = trees[rand() % trees.size()];
        return action_result::succeeded;
    }
    case action_type::goto_target: {
        if (!state.valid(agent.target))
        {
            debug_print("goto target lost");
            return action_result::failed;
        }

        auto target_pos = state.get<position>(agent.target);
        auto target_offset = (target_pos - agent_position);
        auto target_dist = target_offset.magnitude();
        if (target_dist < 0.1f)
            return action_result::succeeded;

        agent_position += (target_offset / target_dist) * std::min(6.f, target_dist);
        return action_result::in_progress;
    }
    case action_type::kill_tree: {
        if (!state.valid(agent.target) || !state.has<tree>(agent.target))
        {
            debug_print("kill target lost");
            return action_result::failed;
        }

        auto tree_stump = al_load_bitmap("data/tree_stump.png");
        state.replace<sprite>(agent.target, tree_stump);
        state.remove<tree>(agent.target);

        return action_result::succeeded;
    }
    default:
        panic();
    }
}

void update_ai(entt::registry& state) {
    auto& global_data = state.ctx<ai_global_data>();

    global_data.ai_agent_created.each([&](entt::entity e) {
        state.assign<ai_agent_system_data>(e);
        debug_print("Assigned system data to %u", (uint32_t)e);
    });

    state.view<ai_agent, ai_agent_system_data, position>().each([&state](ai_agent& agent, ai_agent_system_data& system_data, position& pos) {
        if (system_data.current_action.type != action_type::none) {
            auto result = execute_action(state, agent, pos, system_data.current_action);
            switch (result) {
            case action_result::in_progress:
                return;

            case action_result::failed:
                debug_print("Action failed");
                system_data.current_action.type = action_type::none;
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