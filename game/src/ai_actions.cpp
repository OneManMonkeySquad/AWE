#include "pch.h"
#include "ai_actions.h"
#include "ai.h"


bool goto_target(scene& scene, action_context& ctx) {
	if (!scene.registry.valid(ctx.agent.target))
		return false;

	auto target_transform = scene.registry.try_get<component::transform>(ctx.agent.target);
	if (target_transform == nullptr)
		return false;

	auto& tr = scene.registry.get<component::transform>(ctx.agent_entity);

	const auto diff = (target_transform->position - tr.position);
	const auto dist = diff.magnitude();

	const auto move_speed = 6;
	tr.position += dist > move_speed ? (diff / dist) * move_speed : diff;

	return dist < 3;
}


action_result hunt_deer(scene& scene, action_context& ctx) {
	auto target_invalid = !scene.registry.valid(ctx.agent.target) || !scene.registry.all_of<component::animal>(ctx.agent.target);
	if (target_invalid) {
		auto deers = scene.registry.view<component::animal>();
		if (deers.empty())
			return action_result::failed;

		ctx.agent.target = deers[rand() % deers.size()];
	}

	if (!goto_target(scene, ctx))
		return action_result::in_progress;

	kill_deer(scene, ctx.agent.target);
	return action_result::succeeded;
}


action_result pickup_meat(scene& scene, action_context& ctx) {
	auto target_invalid = !scene.registry.valid(ctx.agent.target) || !scene.registry.all_of<component::item>(ctx.agent.target);
	if (target_invalid || scene.registry.get<component::item>(ctx.agent.target).type != item_type::meat) {
		auto items = scene.registry.view<component::item, component::transform>();
		if (!items)
			return action_result::failed;

		std::vector<entt::entity> meats;
		for (auto item_entity : items) {
			auto it = items.get<component::item>(item_entity);
			if (it.type == item_type::meat) {
				meats.push_back(item_entity);
			}
		}
		if (meats.empty())
			return action_result::failed;

		ctx.agent.target = meats[rand() % meats.size()];
	}

	if (!goto_target(scene, ctx))
		return action_result::in_progress;

	inventory_add_item(scene, ctx.agent_entity, ctx.agent.target);
	return action_result::succeeded;
}


action_result eat_meat(scene& scene, action_context& ctx) {
	auto agent = scene.get(ctx.agent_entity);

	auto& inv = scene.registry.get<component::inventory>(ctx.agent_entity);

	if (!inventory_has_item_of_type(scene, ctx.agent_entity, item_type::meat))
		return action_result::failed;

	auto item = inventory_remove_item_of_type(scene, ctx.agent_entity, item_type::meat);
	if (item == entt::null)
		return action_result::failed;

	scene.registry.destroy(item);
	ctx.agent.hunger = 0;

	return action_result::succeeded;
}