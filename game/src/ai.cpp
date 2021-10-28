
#include "pch.h"
#include "ai.h"
#include "game.h"
#include "ai_actions.h"


enum class ai_property {
	is_hungry,
	can_see_meat,
	has_meat
};

struct ai_world_property {
	ai_property key;
	union {
		bool as_bool;
		float as_float;
		int as_int;
	};
};

using ai_world_state = std::vector<ai_world_property>;

struct ai_global_actions {
	std::vector< const char* > action_names;
	std::vector< entt::delegate< action_result(scene&, action_context&) > > impl_start;
	std::vector< entt::delegate< action_result(scene&, action_context&) > > impl_update;
	std::vector< ai_world_state > preconditions;
	std::vector< ai_world_state > effects;
};

constexpr int AI_LOW_LEVEL = 0;
constexpr int AI_HIGH_LEVEL = 1;

struct ai_global_data {
	entt::observer ai_agent_created;
	ai_global_actions actions[2];
};


using plan = std::vector<uint8_t>;

namespace component {
	struct ai_agent_system_data {
		plan plans[2];
	};
}

plan create_plan(const ai_global_actions& actions, ai_world_state curr_state, ai_world_state target_state);


ai_world_property make_world_property(ai_property key, bool value) {
	ai_world_property prop;
	prop.key = key;
	prop.as_bool = value;
	return prop;
}


ai_world_property make_world_property(ai_property key, float value) {
	ai_world_property prop;
	prop.key = key;
	prop.as_float = value;
	return prop;
}


ai_world_property make_world_property(ai_property key, int value) {
	ai_world_property prop;
	prop.key = key;
	prop.as_int = value;
	return prop;
}


void add_goal(ai_global_actions& actions, const char* name, ai_world_state preconditions, ai_world_state effects) {
	actions.action_names.push_back(name);
	actions.preconditions.push_back(preconditions);
	actions.effects.push_back(effects);
}


template<auto FuncUpdateT>
void add_action(ai_global_actions& actions, const char* name, ai_world_state preconditions, ai_world_state effects) {
	actions.action_names.push_back(name);
	actions.impl_update.push_back({ entt::connect_arg<FuncUpdateT> });
	actions.preconditions.push_back(preconditions);
	actions.effects.push_back(effects);
}


void init_ai(scene& scene) {
	auto& global_data = scene.registry.ctx_or_set<ai_global_data>();
	global_data.ai_agent_created.connect(scene.registry, entt::collector.group<component::ai_agent>());


	//////////////////////////////////// Low Level ////////////////////////////////////


	add_action<&hunt_deer>(global_data.actions[AI_LOW_LEVEL], "Hunt Deer",
						   {},
						   { make_world_property(ai_property::can_see_meat, true) }
	);

	add_action<&pickup_meat>(global_data.actions[AI_LOW_LEVEL], "Pickup Meat",
							 { make_world_property(ai_property::can_see_meat, true) },
							 { make_world_property(ai_property::has_meat, true) }
	);

	add_action<&eat_meat>(global_data.actions[AI_LOW_LEVEL], "Eat Meat",
						  { make_world_property(ai_property::has_meat, true) },
						  { make_world_property(ai_property::has_meat, false), make_world_property(ai_property::is_hungry, false) }
	);


	/////////////////////////////////// High Level ////////////////////////////////////

	// Fight hunger
	add_goal(global_data.actions[AI_HIGH_LEVEL], "Hunger",
			 { make_world_property(ai_property::is_hungry, true) },
			 { make_world_property(ai_property::is_hungry, false) }
	);
}




void update_ai(scene& scene) {
	ai_global_data& global_data = scene.registry.ctx<ai_global_data>();

	global_data.ai_agent_created.each([&](entt::entity e) {
		scene.registry.emplace<component::ai_agent_system_data>(e);
	});

	auto ai_agents = scene.registry.view<component::ai_agent, component::ai_agent_system_data, component::transform>();
	for (auto agent_entity : ai_agents) {
		auto& agent = ai_agents.get<component::ai_agent>(agent_entity);
		auto& agent_system_data = ai_agents.get<component::ai_agent_system_data>(agent_entity);
		const auto& agent_transform = scene.registry.get<component::transform>(agent_entity);

		agent.hunger = std::min(agent.hunger + 0.005f, 1.f);
		debug::draw_world_text(scene, agent_transform.position + math::vector2{ 20, 20 }, std::format("hunger={:.2}", agent.hunger));

		{
			std::string desc0;
			auto& plan = agent_system_data.plans[AI_HIGH_LEVEL];
			for (int i = 0; i < plan.size(); ++i) {
				desc0 += global_data.actions[AI_HIGH_LEVEL].action_names[plan[i]];
				if (i < plan.size() - 1) {
					desc0 += ",";
				}
			}
			debug::draw_world_text(scene, agent_transform.position + math::vector2{ 40, 40 }, desc0);
		}


		{
			std::string desc1;
			auto& plan = agent_system_data.plans[AI_LOW_LEVEL];
			for (int i = 0; i < plan.size(); ++i) {
				desc1 += global_data.actions[AI_LOW_LEVEL].action_names[plan[i]];
				if (i < plan.size() - 1) {
					desc1 += ",";
				}
			}
			debug::draw_world_text(scene, agent_transform.position + math::vector2{ 50, 50 }, desc1);
		}


		if (!agent_system_data.plans[AI_LOW_LEVEL].empty()) {
			auto current_action_idx = agent_system_data.plans[AI_LOW_LEVEL][0];
			auto action = global_data.actions[AI_LOW_LEVEL].impl_update[current_action_idx];

			action_context ctx{
				agent,
				agent_entity
			};
			const auto result = action(scene, ctx);
			switch (result) {
			case action_result::in_progress:
				continue;

			case action_result::failed:
				agent_system_data.plans[AI_LOW_LEVEL].clear();
				agent_system_data.plans[AI_HIGH_LEVEL].clear();
				break;

			case action_result::succeeded:
				break;
			}
		}

		// Nächste Aktion
		if (!agent_system_data.plans[AI_LOW_LEVEL].empty()) {
			agent_system_data.plans[AI_LOW_LEVEL].erase(agent_system_data.plans[AI_LOW_LEVEL].begin());
		}
		else {
			if (!agent_system_data.plans[AI_HIGH_LEVEL].empty()) {
				agent_system_data.plans[AI_HIGH_LEVEL].erase(agent_system_data.plans[AI_HIGH_LEVEL].begin());
			}

			ai_world_state curr_state;
			{
				auto hungry = agent.hunger > 0.75f;
				curr_state.push_back(make_world_property(ai_property::is_hungry, hungry));

				bool can_see_meat = false;
				scene.registry.view<component::item>().each([&](auto& item) {
					if (item.type == item_type::meat) {
						can_see_meat = true;
					}
				});

				curr_state.push_back(make_world_property(ai_property::can_see_meat, can_see_meat));

				auto has_meat = inventory_has_item_of_type(scene, agent_entity, item_type::meat);
				curr_state.push_back(make_world_property(ai_property::has_meat, has_meat));
			}

			if (!agent_system_data.plans[AI_HIGH_LEVEL].empty()) {
				auto& target_state = global_data.actions[AI_HIGH_LEVEL].effects[agent_system_data.plans[AI_HIGH_LEVEL][0]];
				//print("=== LOW LEVEL ===");
				agent_system_data.plans[AI_LOW_LEVEL] = create_plan(global_data.actions[AI_LOW_LEVEL], curr_state, target_state);

				auto plan_failed = agent_system_data.plans[AI_LOW_LEVEL].empty();
				if (plan_failed) {
					agent_system_data.plans[AI_HIGH_LEVEL].clear();
				}
			}
			else {
				//print("=== HIGH LEVEL ===");
				agent_system_data.plans[AI_HIGH_LEVEL] = create_plan(global_data.actions[AI_HIGH_LEVEL], curr_state, { make_world_property(ai_property::is_hungry, false) });

				auto planning_failed = agent_system_data.plans[AI_HIGH_LEVEL].empty();
				if (!planning_failed) {
					auto& target_state = global_data.actions[AI_HIGH_LEVEL].effects[agent_system_data.plans[AI_HIGH_LEVEL][0]];

					//print("=== LOW LEVEL ===");
					agent_system_data.plans[AI_LOW_LEVEL] = create_plan(global_data.actions[AI_LOW_LEVEL], curr_state, target_state);
				}
			}
		}
	}
}


bool satisfies_one_of(ai_world_state effects, ai_world_state state) {
	for (auto& state_prop : state) {
		for (auto& effect_prop : effects) {
			if (effect_prop.key == state_prop.key) {
				switch (effect_prop.key) {
				case ai_property::is_hungry:
					if (!effect_prop.as_bool && state_prop.as_bool)
						return true;

					break;

				default:
					if (effect_prop.as_bool && !state_prop.as_bool)
						return true;

					break;
				}

			}
		}
	}
	return false;
}

bool satisfies_all_of(ai_world_state effects, ai_world_state state) {
	for (auto& state_prop : state) {
		bool found = false;
		for (auto& effect_prop : effects) {
			if (effect_prop.key == state_prop.key) {
				if (effect_prop.as_bool != state_prop.as_bool)
					return false;

				found = true;
				break;
			}
		}

		if (!found)
			return false;
	}

	return true;
}

ai_world_state apply(ai_world_state effects, ai_world_state state) {
	ai_world_state result = state;

	for (auto& effect_prop : effects) {
		bool applied = false;
		for (auto& result_prop : result) {
			if (result_prop.key == effect_prop.key) {
				result_prop.as_bool = effect_prop.as_bool;

				applied = true;
			}
		}

		if (applied)
			continue;

		result.push_back(effect_prop);
	}

	return result;
}

plan create_plan(const ai_global_actions& actions, ai_world_state curr_state, ai_world_state target_state) {
	std::vector< std::pair<plan, ai_world_state> > todo;
	todo.emplace_back(plan{}, curr_state);

	int num_itr = 0;
	while (!todo.empty()) {
		++num_itr;

		auto plan_until_now = todo.back().first;
		auto state_until_now = todo.back().second;
		todo.pop_back();

		if (satisfies_all_of(state_until_now, target_state)) {
			std::string desc;
			{
				for (auto idx : plan_until_now) {
					desc += actions.action_names[idx];
					desc += ",";
				}
			}

			//print("found plan after {} iterations: {}", num_itr, desc.c_str());
			return plan_until_now;
		}

		for (int i = 0; i < actions.action_names.size(); ++i) {
			//print("lets see if {} satisfies state_until_now", actions.action_names[i]);

			if (!satisfies_all_of(state_until_now, actions.preconditions[i])) {
				//print("preconiditions not satisfied!");
				continue;
			}

			if (satisfies_one_of(actions.effects[i], state_until_now)) {
				//print("yep");

				auto state_after_action = apply(actions.effects[i], state_until_now);

				auto new_plan = plan_until_now;
				new_plan.push_back(i);
				todo.emplace_back(new_plan, state_after_action);
			}
			else {
				//print("nope");
			}
		}
	}

	//print("failed to find plan after {} iterations", num_itr);
	return {};
}