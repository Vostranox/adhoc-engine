#include "SceneHierarchyPanel.hpp"
#include "../EditorActions.hpp"
#include "../EditorContext.hpp"
#include "../IconFontCppHeaders/IconFontAwesome5.hpp"
#include "../Theme.hpp"
#include "MainMenuBar.hpp"

#include <Scene/Components.hpp>
#include <Scene/Scene.hpp>
#include <Scripting/Script.hpp>

#include <ImGui/imgui.h>

#include <string>
#include <utility>

namespace adh {
    static const char* GetIcon(ecs::World& world, ecs::Entity entity) {
        if (world.has_component<Camera3D>(entity) || world.has_component<Camera2D>(entity)) {
            return ICON_FA_VIDEO;
        }
        if (world.has_component<Light>(entity)) {
            return ICON_FA_LIGHTBULB;
        }
        if (world.has_component<Skybox>(entity)) {
            return ICON_FA_CLOUD;
        }
        if (world.has_component<ParticleEmitter>(entity)) {
            return ICON_FA_FIRE;
        }
        if (world.has_component<Mesh>(entity)) {
            return ICON_FA_CUBE;
        }
        if (world.has_component<Script>(entity)) {
            return ICON_FA_FILE_CODE;
        }
        return ICON_FA_DOT_CIRCLE;
    }

    SceneHierarchyPanel::SceneHierarchyPanel() noexcept : Panel{ "Scene Hierarchy" } {
    }

    void SceneHierarchyPanel::OnImGui(EditorContext& context) {
        if (ImGui::Button(ICON_FA_PLUS)) {
            ImGui::OpenPopup("Create");
        }
        ImGui::SetItemTooltip("Create an entity");
        if (ImGui::BeginPopup("Create")) {
            DrawCreateEntityItems(context);
            ImGui::EndPopup();
        }

        ImGui::BeginChild("Entities");
        const ecs::Entity lastNameField{ std::exchange(m_NameFieldEntity, ecs::NULL_ENTITY) };
        for (const ecs::Entity entity : context.scene->GetEntities()) {
            if (entity == context.renamedEntity) {
                if (entity != lastNameField) {
                    ImGui::SetKeyboardFocusHere();
                }
                DrawNameField(context, entity);
            } else {
                DrawEntity(context, entity);
            }
        }

        if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            actions::Deselect(context);
        }

        if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
            DrawCreateEntityItems(context);
            ImGui::EndPopup();
        }
        ImGui::EndChild();
    }

    void SceneHierarchyPanel::DrawEntity(EditorContext& context, ecs::Entity entity) {
        ecs::World& world{ context.scene->GetWorld() };
        const bool isSelected{ entity == context.selectedEntity };
        const std::string label{ std::string{ GetIcon(world, entity) } + "  " + (world.has_component<Tag>(entity) ? world.get<Tag>(entity).tag : "") };

        ImGui::PushID(reinterpret_cast<void*>(entity));
        if (isSelected) {
            ImGui::PushStyleColor(ImGuiCol_Header, theme::accent);
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, theme::accentHovered);
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, theme::accentActive);
        }
        if (ImGui::Selectable("##Row", isSelected, ImGuiSelectableFlags_AllowDoubleClick)) {
            actions::SelectEntity(context, entity);
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                context.renamedEntity = entity;
            }
        }
        if (isSelected) {
            ImGui::PopStyleColor(3);
        }

        if (ImGui::BeginPopupContextItem()) {
            actions::SelectEntity(context, entity);
            DrawMenuItems(context, actions::entityActions);
            ImGui::EndPopup();
        }

        ImGui::SameLine(0.0f, 0.0f);
        ImGui::TextUnformatted(label.c_str());
        ImGui::PopID();
    }

    void SceneHierarchyPanel::DrawNameField(EditorContext& context, ecs::Entity entity) {
        m_NameFieldEntity = entity;
        ecs::World& world{ context.scene->GetWorld() };
        if (!world.has_component<Tag>(entity)) {
            world.add<Tag>(entity, Tag{});
        }
        std::string& name{ world.get<Tag>(entity).tag };

        actions::FieldsEdit edit{ context, entity, "Rename " + name };
        char buffer[256]{};
        name.copy(buffer, sizeof(buffer) - 1);
        ImGui::PushID(reinterpret_cast<void*>(entity));
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::InputText("##Name", buffer, sizeof(buffer), ImGuiInputTextFlags_AutoSelectAll)) {
            name = buffer;
        }
        if (ImGui::IsItemDeactivated()) {
            context.renamedEntity = ecs::NULL_ENTITY;
        }
        ImGui::PopID();
    }

    void SceneHierarchyPanel::OnHidden(EditorContext& context) {
        context.renamedEntity = ecs::NULL_ENTITY;
        m_NameFieldEntity     = ecs::NULL_ENTITY;
    }
} // namespace adh
