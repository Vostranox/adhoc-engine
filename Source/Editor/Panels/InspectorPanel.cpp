#include "InspectorPanel.hpp"
#include "../EditorActions.hpp"
#include "../EditorContext.hpp"
#include "../IconFontCppHeaders/IconFontAwesome5.hpp"
#include "../Shell.hpp"
#include "../Theme.hpp"
#include "../Widgets.hpp"
#include "ComponentEditors.hpp"

#include <Scene/Components.hpp>
#include <Scene/Scene.hpp>
#include <Scripting/Script.hpp>
#include <Utf8.hpp>

#include <ImGui/imgui.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <string>

namespace adh {
    static void DrawComponent(EditorContext& context, const ComponentEditor& editor, ecs::Entity entity) {
        const std::string name{ editor.name };
        ImGui::PushID(editor.name);

        constexpr ImGuiTreeNodeFlags headerFlags{ ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth |
                                                  ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_NoTreePushOnOpen };
        const float right{ ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x };
        ImGui::PushFont(theme::GetFonts().bold, 0.0f);
        const bool isOpen{ ImGui::TreeNodeEx("##Header", headerFlags, "%s  %s", editor.icon, editor.name) };
        ImGui::PopFont();
        ImGui::OpenPopupOnItemClick("Menu", ImGuiPopupFlags_MouseButtonRight);

        const float buttonWidth{ ImGui::GetFrameHeight() };
        ImGui::SameLine(right - buttonWidth);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f });
        if (ImGui::Button(ICON_FA_ELLIPSIS_V, ImVec2{ buttonWidth, 0.0f })) {
            ImGui::OpenPopup("Menu");
        }
        ImGui::PopStyleColor();

        bool removeComponent = false;
        if (ImGui::BeginPopup("Menu")) {
            if (ImGui::MenuItem("Reset", nullptr, false, editor.reset != nullptr)) {
                actions::FinishEdit(context);
                editor.reset(context.scene->GetWorld(), entity);
                actions::CommitEdit(context, "Reset " + name);
            }
            if (ImGui::MenuItem("Remove")) {
                removeComponent = true;
            }
            ImGui::EndPopup();
        }

        if (isOpen) {
            actions::FieldsEdit edit{ context, entity, "Edit " + name };
            if (widgets::BeginProperties("Properties")) {
                editor.draw(context, entity);
                widgets::EndProperties();
            }
        }

        if (removeComponent) {
            actions::FinishEdit(context);
            editor.remove(context.scene->GetWorld(), entity);
            actions::CommitEdit(context, "Remove " + name);
        }
        ImGui::PopID();
        ImGui::Spacing();
    }

    static void DrawHint(const char* text) {
        const float width{ ImGui::GetContentRegionAvail().x };
        const float textWidth{ std::min(ImGui::CalcTextSize(text).x, width) };
        ImGui::Spacing();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (width - textWidth) / 2.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + textWidth);
        ImGui::TextUnformatted(text);
        ImGui::PopTextWrapPos();
        ImGui::PopStyleColor();
    }

    static bool DrawAddItem(const std::string& label, const char* reason) {
        return ImGui::MenuItem(label.c_str(), reason, false, reason == nullptr);
    }

    static void AddRigidBody(EditorContext& context, ecs::Entity entity, PhysicsBodyType type, PhysicsColliderShape shape) {
        ecs::World& world{ context.scene->GetWorld() };
        auto& rigidBody{ world.add<RigidBody>(entity, RigidBody{}) };
        auto& transform{ world.get<Transform>(entity) };

        Mesh* meshPtr{ nullptr };
        if ((shape == PhysicsColliderShape::eMesh || shape == PhysicsColliderShape::eConvexMesh) && world.has_component<Mesh>(entity)) {
            meshPtr = &world.get<Mesh>(entity);
        }
        rigidBody.Create(static_cast<std::uint64_t>(entity),
                         0.5f,
                         0.5f,
                         1.0f,
                         type,
                         1.0f,
                         true,
                         false,
                         true,
                         shape,
                         PhysicsColliderType::eCollider,
                         transform.scale,
                         1.0f,
                         0.5f,
                         meshPtr);
        physx::PxTransform t;
        t.p = physx::PxVec3{ transform.translate.x, transform.translate.y, transform.translate.z };
        Quaternion<float> qq(transform.rotation);
        physx::PxQuat q(qq.x, qq.y, qq.z, qq.w);
        t.q = q;
        rigidBody.actor->setGlobalPose(t);
        transform.q = qq;
    }

    static bool DrawRigidBodyItems(EditorContext& context, ecs::Entity entity) {
        struct Shape {
            const char* name;
            PhysicsColliderShape shape;
            bool needsMesh;
        };
        constexpr Shape shapes[]{
            { "Box Collider", PhysicsColliderShape::eBox, false },
            { "Sphere Collider", PhysicsColliderShape::eSphere, false },
            { "Capsule Collider", PhysicsColliderShape::eCapsule, false },
            { "Mesh Collider", PhysicsColliderShape::eMesh, true },
            { "Convex Mesh Collider", PhysicsColliderShape::eConvexMesh, true },
        };
        const bool hasMesh{ context.scene->GetWorld().has_component<Mesh>(entity) };
        bool added{};
        for (const auto& [typeName, type] : { std::pair{ "Dynamic", PhysicsBodyType::eDynamic }, std::pair{ "Static", PhysicsBodyType::eStatic } }) {
            if (ImGui::BeginMenu(typeName)) {
                for (const Shape& shape : shapes) {
                    if (DrawAddItem(shape.name, shape.needsMesh && !hasMesh ? "Needs a Mesh" : nullptr)) {
                        actions::FinishEdit(context);
                        AddRigidBody(context, entity, type, shape.shape);
                        added = true;
                    }
                }
                ImGui::EndMenu();
            }
        }
        return added;
    }

    InspectorPanel::InspectorPanel() noexcept : Panel{ "Inspector" } {
    }

    void InspectorPanel::OnImGui(EditorContext& context) {
        if (m_NewScriptEntity != context.selectedEntity) {
            m_NewScriptEntity = ecs::NULL_ENTITY;
        }

        if (context.scene->GetWorld().is_valid(context.selectedEntity)) {
            DrawComponents(context, context.selectedEntity);
        } else {
            DrawHint("Select an entity in the Hierarchy or in the Scene view to see its components here");
        }
    }

    void InspectorPanel::DrawComponents(EditorContext& context, ecs::Entity entity) {
        ecs::World& world{ context.scene->GetWorld() };
        if (world.has_component<Tag>(entity)) {
            auto& tagComp = world.get<Tag>(entity);
            auto& tag     = tagComp.tag;

            actions::FieldsEdit edit{ context, entity, "Rename " + tag };
            char buffer[256]{};
            tag.copy(buffer, sizeof(buffer) - 1);
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
                tag = std::string(buffer);
            }
        }

        ImGui::Spacing();

        for (const ComponentEditor& editor : GetComponentEditors()) {
            if (editor.draw && editor.has(world, entity)) {
                DrawComponent(context, editor, entity);
            }
        }

        AddComponent(context, entity);
    }

    void InspectorPanel::AddComponent(EditorContext& context, ecs::Entity entity) {
        ecs::World& world{ context.scene->GetWorld() };

        if (ImGui::Button(ICON_FA_PLUS "  Add Component", ImVec2{ -FLT_MIN, 0.0f })) {
            m_ComponentFilter.Clear();
            ImGui::OpenPopup("AddComponent");
        }
        const ImVec2 buttonMin{ ImGui::GetItemRectMin() };
        const ImVec2 buttonMax{ ImGui::GetItemRectMax() };
        ImGui::SetNextWindowPos(ImVec2{ buttonMin.x, buttonMax.y }, ImGuiCond_Appearing);
        ImGui::SetNextWindowSize(ImVec2{ buttonMax.x - buttonMin.x, 0.0f });

        const char* added{};
        bool isScriptChosen{};
        if (ImGui::BeginPopup("AddComponent")) {
            if (ImGui::IsWindowAppearing()) {
                ImGui::SetKeyboardFocusHere();
            }
            widgets::SearchField(m_ComponentFilter);
            ImGui::Separator();

            bool isAnyShown{};
            for (const ComponentEditor& editor : GetComponentEditors()) {
                if (!m_ComponentFilter.PassFilter(editor.name)) {
                    continue;
                }
                isAnyShown = true;
                const std::string label{ std::string{ editor.icon } + "  " + editor.name };
                const char* reason{ editor.has(world, entity) ? "Already added"
                                    : editor.whyNotAddable    ? editor.whyNotAddable(context, entity)
                                                              : nullptr };
                if (reason) {
                    DrawAddItem(label, reason);
                    continue;
                }
                switch (editor.addKind) {
                case AddKind::eDefault:
                    if (DrawAddItem(label, nullptr)) {
                        actions::FinishEdit(context);
                        editor.add(context, entity);
                        added = editor.name;
                    }
                    break;
                case AddKind::eRigidBody:
                    if (ImGui::BeginMenu(label.c_str())) {
                        if (DrawRigidBodyItems(context, entity)) {
                            added = editor.name;
                        }
                        ImGui::EndMenu();
                    }
                    break;
                case AddKind::eScript:
                    isScriptChosen = DrawAddItem(label + "...", nullptr);
                    break;
                }
            }
            if (!isAnyShown) {
                ImGui::TextDisabled("No component matches");
            }

            ImGui::EndPopup();
        }
        if (added) {
            actions::CommitEdit(context, std::string{ "Add " } + added);
        }

        if (isScriptChosen) {
            m_NewScriptEntity = entity;
            ImGui::OpenPopup("Input new script name:");
        }
        if (m_NewScriptEntity != ecs::NULL_ENTITY) {
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

            if (ImGui::BeginPopupModal("Input new script name:", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Separator();
                ImGui::InputText("##Tag", m_NewScriptName, sizeof(m_NewScriptName));
                std::string buf{ m_NewScriptName };
                if (ImGui::Button("OK", ImVec2(120, 0))) {
                    if (buf.empty()) {
                        buf = "Default";
                    }
                    ImGui::CloseCurrentPopup();
                    std::memset(m_NewScriptName, 0, sizeof(m_NewScriptName));

                    const std::filesystem::path script{ context.paths.scripts / PathFromUtf8(buf + ".lua") };
                    if (shell::CopyIfMissing(context.paths.scriptTemplate, script)) {
                        shell::Open(script);
                        if (world.is_valid(m_NewScriptEntity) && !world.has_component<Script>(m_NewScriptEntity)) {
                            actions::FinishEdit(context);
                            world.add<Script>(m_NewScriptEntity, Script{ context.scene->GetState(), ToUtf8(script) });
                            actions::CommitEdit(context, "Add Script");
                        }
                    }
                    m_NewScriptEntity = ecs::NULL_ENTITY;
                }
                ImGui::SetItemDefaultFocus();
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                    ImGui::CloseCurrentPopup();
                    m_NewScriptEntity = ecs::NULL_ENTITY;
                }
                ImGui::EndPopup();
            } else {
                m_NewScriptEntity = ecs::NULL_ENTITY;
            }
        }
    }
} // namespace adh
