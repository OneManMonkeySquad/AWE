
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
	entt::meta<component::inventory>()
		.type("inventory"_hs)
		.prop("hierarchy_tag"_hs, (const char*)"Inventory")

		.ctor();


	entt::meta<component::ai_agent>()
		.type("ai_agent"_hs)
		.prop("hierarchy_tag"_hs, (const char*)"AIAgent")

		.ctor();


	entt::meta<component::item>()
		.type("item"_hs)
		.prop("hierarchy_tag"_hs, (const char*)"Item")

		.ctor();


	entt::meta<component::tree>()
		.type("tree"_hs)
		.prop("hierarchy_tag"_hs, (const char*)"Tree")

		.ctor();


	entt::meta<component::animal>()
		.type("deer"_hs)
		.prop("hierarchy_tag"_hs, (const char*)"Deer")

		.ctor();


	entt::meta<component::animal_edible>()
		.type("animal_edible"_hs)
		.prop("hierarchy_tag"_hs, (const char*)"Animal Edible")

		.ctor();


	entt::meta<component::pawn>()
		.type("pawn"_hs)
		.prop("hierarchy_tag"_hs, (const char*)"Pawn")

		.ctor()

		.data<&component::pawn::client_idx>("client_idx"_hs).prop("name"_hs, (const char*)"client_idx");


	entt::meta<math::vector2>()
		.func<&get<math::vector2>, entt::as_ref_t>("get"_hs)
		.func<&set<math::vector2>>("set"_hs)

		.type("vector2"_hs)

		.ctor()
		.data<&math::vector2::x>("x"_hs).prop("name"_hs, (const char*)"x")
		.data<&math::vector2::y>("y"_hs).prop("name"_hs, (const char*)"y");


	entt::meta<component::velocity>()
		.func<&get<component::velocity>, entt::as_ref_t>("get"_hs)
		.func<&set<component::velocity>>("set"_hs)

		.type("velocity"_hs)

		.ctor()
		.data<&component::velocity::dx>("dx"_hs).prop("name"_hs, (const char*)"dx")
		.data<&component::velocity::dy>("dy"_hs).prop("name"_hs, (const char*)"dy");


	entt::meta<component::transform>()
		.func<&get<component::transform>, entt::as_ref_t>("get"_hs)
		.func<&set<component::transform>>("set"_hs)

		.type("transform"_hs)
		.ctor()

		.data<&component::transform::position>("position"_hs).prop("name"_hs, (const char*)"position")
		.data<&component::transform::angle>("angle"_hs).prop("name"_hs, (const char*)"angle");


	entt::meta<bitmap_id>()
		.func<&get<bitmap_id>, entt::as_ref_t>("get"_hs)
		.func<&set<bitmap_id>>("set"_hs)

		.type("bitmap_id"_hs)
		.data<&bitmap_id::resource_hash>("resource_hash"_hs).prop("name"_hs, (const char*)"resource_hash");


	entt::meta<component::sprite_instance>()
		.func<&get<component::sprite_instance>, entt::as_ref_t>("get"_hs)
		.func<&set<component::sprite_instance>>("set"_hs)

		.type("sprite_instance"_hs)
		.data<&component::sprite_instance::bitmap>("bitmap"_hs).prop("name"_hs, (const char*)"bitmap");
}