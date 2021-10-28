
#include "pch.h"
#include "editor.h"
#include "math.h"
#include "game.h"

using namespace entt::literals;

namespace {
	entt::entity selected_entity = entt::null;
	entt::entity hovered_entity = entt::null;

	void draw_hierarchy(const scene& scene) {
		hovered_entity = entt::null;

		defer{ ImGui::End(); };
		if (!ImGui::Begin("Hierarchy", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysVerticalScrollbar))
			return;

		scene.registry.each([&](const auto entity) {
			auto entityName = std::to_string((uint32_t)entity);

			scene.registry.visit(entity, [&](entt::type_info type_info) {
				const auto type = entt::resolve(type_info);
				if (type) {
					auto tag = type.prop("hierarchy_tag"_hs);
					if (tag) {
						entityName += " [";
						entityName += tag.value().cast<const char*>();
						entityName += "]";
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
		else if (auto val = value.try_cast<int>(); val) {
			ImGui::LabelText(name, "%d", val);
		}
		else if (auto val = value.try_cast<uint16_t>(); val) {
			ImGui::LabelText(name, "%u", val);
		}
		else if (auto val = value.try_cast<uint32_t>(); val) {
			ImGui::LabelText(name, "%u", val);
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

	void draw_inspector(const scene& scene) {
		defer{ ImGui::End(); };
		if (!ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoCollapse))
			return;

		if (!scene.registry.valid(selected_entity))
			return;

		scene.registry.visit(selected_entity, [&](entt::type_info type_info) {
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
					const auto component = getter.invoke({}, entt::forward_as_meta((entt::registry&)scene.registry), entt::forward_as_meta(selected_entity));
					draw_inspector_data(component, "");
				}
			}

			ImGui::Separator();
		});
	}
}

void debugger_draw(scene& scene) {
	if (ImGui::BeginMainMenuBar()) {
		defer{ ImGui::EndMainMenuBar(); };

		if (ImGui::BeginMenu("Menu")) {
			defer{ ImGui::EndMenu(); };

			if (ImGui::MenuItem("Main menu bar", nullptr, nullptr)) {}
		}
	}

	ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

	draw_hierarchy(scene);
	draw_inspector(scene);

	if (scene.registry.valid(selected_entity)) {
		const auto tr = scene.registry.try_get<component::transform>(selected_entity);
		if (tr) {
			const auto color = math::color::green;
			debug::draw_world_line(scene, tr->position + math::vector2{ 20, -20 }, tr->position + math::vector2{ 20, 20 }, color);
			debug::draw_world_line(scene, tr->position + math::vector2{ 20, 20 }, tr->position + math::vector2{ -20, 20 }, color);
			debug::draw_world_line(scene, tr->position + math::vector2{ -20, 20 }, tr->position + math::vector2{ -20, -20 }, color);
			debug::draw_world_line(scene, tr->position + math::vector2{ -20, -20 }, tr->position + math::vector2{ 20, -20 }, color);
		}
	}

	if (scene.registry.valid(hovered_entity)) {
		const auto tr = scene.registry.try_get<component::transform>(hovered_entity);
		if (tr) {
			const auto color = math::color{ 255, 255, 255, 100 };
			debug::draw_world_line(scene, tr->position + math::vector2{ 20, -20 }, tr->position + math::vector2{ 20, 20 }, color);
			debug::draw_world_line(scene, tr->position + math::vector2{ 20, 20 }, tr->position + math::vector2{ -20, 20 }, color);
			debug::draw_world_line(scene, tr->position + math::vector2{ -20, 20 }, tr->position + math::vector2{ -20, -20 }, color);
			debug::draw_world_line(scene, tr->position + math::vector2{ -20, -20 }, tr->position + math::vector2{ 20, -20 }, color);
		}
	}
}