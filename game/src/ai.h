#pragma once

#include "ai_actions.h"

struct ai_agent {
    entt::entity target;
    float hunger;
};

void init_ai(entt::registry& state);
void update_ai(entt::registry& state);
