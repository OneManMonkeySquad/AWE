
#include "pch.h"
#include "editor.h"
#include "defer.h"
#include "math.h"
#include "utils.h"
#include "game.h"

using namespace entt::literals;

namespace {
	entt::entity selected_entity = entt::null;
	entt::entity hovered_entity = entt::null;

	void draw_hierarchy(const entt::registry& game_state) {
		hovered_entity = entt::null;

		defer{ ImGui::End(); };
		if (!ImGui::Begin("Hierarchy", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysVerticalScrollbar))
			return;

		game_state.each([&](const auto entity) {
			auto entityName = std::to_string((uint32_t)entity);

			game_state.visit(entity, [&](entt::type_info type_info) {
				const auto type = entt::resolve(type_info);
				if (type) {
					auto tag = type.prop("hierarchy_tag"_hs);
					if (tag) {
						entityName += " ";
						entityName += tag.value().cast<const char*>();
					}
				}
			});

			int flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanAvailWidth;
			if (selected_entity == entity) {
				flags |= ImGuiTreeNodeFlags_Selected;
			}
			if (ImGui::TreeNodeEx(entityName.c_str(), flags)) {
				defer{ ImGui::TreePop(); };
			}

			if (ImGui::IsItemClicked()) {
				selected_entity = entity;
			}
			if (ImGui::IsItemHovered()) {
				hovered_entity = entity;
			}
		});
	}

	void draw_inspector_data(entt::meta_any value, const char* name) {
		if (auto val = value.try_cast<float>(); val) {
			ImGui::DragFloat(name, val, 1, 0, 0, "%.2f");
		}
		else if (auto val = value.try_cast<uint16_t>(); val) {
			ImGui::DragInt(name, (int*)val, 1, 0, 0, "%.2f");
		}
		else if (auto val = value.try_cast<math::vector2>(); val) {
			ImGui::DragFloat2(name, (float*)val, 1, 0, 0, "%.2f");
		}
		else {
			for (auto data : value.type().data()) {
				auto current_data = data.get(value);

				auto name = "";
				if (auto prop = data.prop("name"_hs); prop) {
					name = prop.value().cast<const char*>();
				}

				draw_inspector_data(current_data, name);
			}
		}
	}

	void draw_inspector(const entt::registry& game_state) {
		defer{ ImGui::End(); };
		if (!ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoCollapse))
			return;

		if (!game_state.valid(selected_entity))
			return;

		game_state.visit(selected_entity, [&](entt::type_info type_info) {
			auto componentName = std::string{ type_info.name() };
			if (componentName.starts_with("struct ")) {
				componentName = componentName.substr(7);
			}

			if (!ImGui::CollapsingHeader(componentName.data(), ImGuiTreeNodeFlags_DefaultOpen))
				return;

			ImGui::Indent();
			defer{ ImGui::Unindent(); };

			const auto type = entt::resolve(type_info);
			if (type) {
				auto getter = type.func("get"_hs);
				if (getter) {
					const auto component = getter.invoke({}, entt::forward_as_meta((entt::registry&)game_state), entt::forward_as_meta(selected_entity));
					draw_inspector_data(component, "");
				}
			}

			ImGui::Separator();
		});
	}
}

void draw_debugger(entt::registry& game_state) {
	if (ImGui::BeginMainMenuBar()) {
		defer{ ImGui::EndMainMenuBar(); };

		if (ImGui::BeginMenu("Menu")) {
			defer{ ImGui::EndMenu(); };

			if (ImGui::MenuItem("Main menu bar", nullptr, nullptr)) {}
		}
	}

	ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

	draw_hierarchy(game_state);
	draw_inspector(game_state);

	if (game_state.valid(selected_entity)) {
		const auto tr = game_state.try_get<transform>(selected_entity);
		if (tr) {
			const auto color = math::color::green;
			debug::draw_world_line(game_state, tr->position + math::vector2{ 20, -20 }, tr->position + math::vector2{ 20, 20 }, color);
			debug::draw_world_line(game_state, tr->position + math::vector2{ 20, 20 }, tr->position + math::vector2{ -20, 20 }, color);
			debug::draw_world_line(game_state, tr->position + math::vector2{ -20, 20 }, tr->position + math::vector2{ -20, -20 }, color);
			debug::draw_world_line(game_state, tr->position + math::vector2{ -20, -20 }, tr->position + math::vector2{ 20, -20 }, color);
		}
	}

	if (game_state.valid(hovered_entity)) {
		const auto tr = game_state.try_get<transform>(hovered_entity);
		if (tr) {
			const auto color = math::color{ 255, 255, 255, 100 };
			debug::draw_world_line(game_state, tr->position + math::vector2{ 20, -20 }, tr->position + math::vector2{ 20, 20 }, color);
			debug::draw_world_line(game_state, tr->position + math::vector2{ 20, 20 }, tr->position + math::vector2{ -20, 20 }, color);
			debug::draw_world_line(game_state, tr->position + math::vector2{ -20, 20 }, tr->position + math::vector2{ -20, -20 }, color);
			debug::draw_world_line(game_state, tr->position + math::vector2{ -20, -20 }, tr->position + math::vector2{ 20, -20 }, color);
		}
	}
}