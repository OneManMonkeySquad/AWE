#pragma once

struct ai_agent {
    entt::entity target;
};

void init_ai(entt::registry& state);
void update_ai(entt::registry& state);
