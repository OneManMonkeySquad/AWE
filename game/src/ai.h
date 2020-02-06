#pragma once

#include "ai_actions.h"

struct ai_agent {
    entt::entity target;
};

struct ai_global_data {
    entt::observer ai_agent_created;
    std::vector< entt::delegate< action_result(entt::registry&, action_context&) > > actions;
};

using plan = std::vector<action>;

struct ai_agent_system_data {
    plan current_plan;
    action current_action;
};

void init_ai(entt::registry& state);
void update_ai(entt::registry& state);
