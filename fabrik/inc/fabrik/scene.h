
#pragma once

class entity;
class const_entity;

class scene {
public:
	entt::registry registry;

	entity create_entity();
	entity get(entt::entity entity);
	const_entity get(entt::entity entity) const;

	void clear();
};
