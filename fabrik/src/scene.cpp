#include "pch.h"
#include "scene.h"

entity scene::create_entity() {
	return { *this, registry.create() };
}

entity scene::get(entt::entity entity) {
	return { *this, entity };
}

const_entity scene::get(entt::entity entity) const {
	return { *this, entity };
}

void scene::clear() {
	registry.clear();
}