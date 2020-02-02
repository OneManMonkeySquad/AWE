#pragma once

struct ai_agent {
    entt::entity target;
};

struct wood_resource {
    uint8_t amount;
};

void init_ai(entt::registry& state);
void update_ai(entt::registry& state);
