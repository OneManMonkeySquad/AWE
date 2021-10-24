
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

class entity {
public:
	entity(scene& scene, entt::entity entity)
		: _scene{ scene }
		, _entity{ entity } {
	}

	template<typename T, typename... ArgTs>
	void emplace(ArgTs&&... args) {
		_scene.registry.emplace<T>(_entity, std::forward<ArgTs>(args)...);
	}

	template<typename T>
	T& get() {
		return _scene.registry.get<T>(_entity);
	}

	template<typename T>
	const T& get() const {
		return _scene.registry.get<T>(_entity);
	}

	template<typename T>
	T* try_get() {
		return _scene.registry.try_get<T>(_entity);
	}

	template<typename T>
	const T* try_get() const {
		return _scene.registry.try_get<T>(_entity);
	}

private:
	scene& _scene;
	entt::entity _entity;
};

class const_entity {
public:
	const_entity(const scene& scene, entt::entity entity)
		: _scene{ scene }
		, _entity{ entity } {
	}

	template<typename T>
	const T& get() const {
		return _scene.registry.get<T>(_entity);
	}

	template<typename T>
	const T* try_get() const {
		return _scene.registry.try_get<T>(_entity);
	}

private:
	const scene& _scene;
	entt::entity _entity;
};