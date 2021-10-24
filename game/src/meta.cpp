
#include "pch.h"
#include "meta.h"
#include "math.h"
#include "game.h"
#include "ai.h"

using namespace entt::literals;

template<typename Type>
Type& get(entt::registry& registry, entt::entity entity) {
	return registry.get_or_emplace<Type>(entity);
}

template<typename Type>
Type& set(entt::registry& registry, entt::entity entity, const Type& instance) {
	return registry.emplace_or_replace<Type>(entity, instance);
}



void init_meta() {
	entt::meta<inventory>()
		.type("inventory"_hs)
		.prop("hierarchy_tag"_hs, (const char*)"Inventory")

		.ctor();


	entt::meta<ai_agent>()
		.type("ai_agent"_hs)
		.prop("hierarchy_tag"_hs, (const char*)"AIAgent")

		.ctor();


	entt::meta<item>()
		.type("item"_hs)
		.prop("hierarchy_tag"_hs, (const char*)"Item")

		.ctor();


	entt::meta<tree>()
		.type("tree"_hs)
		.prop("hierarchy_tag"_hs, (const char*)"Tree")

		.ctor();


	entt::meta<deer>()
		.type("deer"_hs)
		.prop("hierarchy_tag"_hs, (const char*)"Deer")

		.ctor();


	entt::meta<pawn>()
		.type("pawn"_hs)
		.prop("hierarchy_tag"_hs, (const char*)"Pawn")

		.ctor();


	entt::meta<math::vector2>()
		.func<&get<transform>, entt::as_ref_t>("get"_hs)
		.func<&set<transform>>("set"_hs)

		.type("vector2"_hs)

		.ctor()
		.data<&math::vector2::x>("x"_hs).prop("name"_hs, (const char*)"x")
		.data<&math::vector2::y>("y"_hs).prop("name"_hs, (const char*)"y");


	entt::meta<velocity>()
		.func<&get<velocity>, entt::as_ref_t>("get"_hs)
		.func<&set<velocity>>("set"_hs)

		.type("velocity"_hs)

		.ctor()
		.data<&velocity::dx>("dx"_hs).prop("name"_hs, (const char*)"dx")
		.data<&velocity::dy>("dy"_hs).prop("name"_hs, (const char*)"dy");


	entt::meta<transform>()
		.func<&get<transform>, entt::as_ref_t>("get"_hs)
		.func<&set<transform>>("set"_hs)

		.type("transform"_hs)
		.ctor()

		.data<&transform::position>("position"_hs).prop("name"_hs, (const char*)"position")
		.data<&transform::angle>("angle"_hs).prop("name"_hs, (const char*)"angle");


	entt::meta<bitmap_id>()
		.func<&get<bitmap_id>, entt::as_ref_t>("get"_hs)
		.func<&set<bitmap_id>>("set"_hs)

		.type("bitmap_id"_hs)
		.data<&bitmap_id::resource_hash>("resource_hash"_hs).prop("name"_hs, (const char*)"resource_hash");


	entt::meta<sprite_instance>()
		.func<&get<sprite_instance>, entt::as_ref_t>("get"_hs)
		.func<&set<sprite_instance>>("set"_hs)

		.type("sprite_instance"_hs)
		.data<&sprite_instance::bitmap>("bitmap"_hs).prop("name"_hs, (const char*)"bitmap");
}