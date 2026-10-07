#include "MainMenuBar.hpp"
#include "../EditorContext.hpp"
#include "../Shell.hpp"
#include "../Theme.hpp"
#include "Panel.hpp"

#include <Scene/Scene.hpp>

#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>

#include <string>

namespace adh {
    static void DrawMenuItem(EditorContext& context, const actions::MenuAction& action, const std::string& what, bool isEnabled) {
        const std::string label{ what.empty() ? std::string{ action.label } : std::string{ action.label } + " " + what };
        if (ImGui::MenuItem(label.c_str(), ImGui::GetKeyChordName(action.shortcut), false, isEnabled)) {
            action.run(context);
        }
    }

    MainMenuBarResult DrawMainMenuBar(EditorContext& context, std::span<Panel* const> panels) {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
                                        ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoBackground;

        float height = ImGui::GetFrameHeight();

        MainMenuBarResult result;
        ImGui::PushStyleColor(ImGuiCol_MenuBarBg, theme::bar);
        if (ImGui::BeginViewportSideBar("##SecondaryMenuBar", NULL, ImGuiDir_Up, height, window_flags)) {
            if (ImGui::BeginMenuBar()) {

                if (ImGui::BeginMenu("File")) {
                    DrawMenuItems(context, actions::fileActions, actions::CanChangeFiles(context));

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Edit")) {
                    const bool canUndo{ actions::CanUndo(context) };
                    const bool canRedo{ actions::CanRedo(context) };
                    DrawMenuItem(context, actions::undoAction, canUndo ? context.history.GetUndoName() : "", canUndo);
                    DrawMenuItem(context, actions::redoAction, canRedo ? context.history.GetRedoName() : "", canRedo);
                    ImGui::Separator();

                    const bool hasSelection{ context.scene->GetWorld().is_valid(context.selectedEntity) };
                    DrawMenuItems(context, actions::entityActions, hasSelection);
                    ImGui::Separator();
                    DrawMenuItems(context, actions::selectionActions, hasSelection);

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Entity")) {
                    DrawCreateEntityItems(context);

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("View")) {
                    for (Panel* panel : panels) {
                        ImGui::MenuItem(panel->GetTitle(), nullptr, &panel->isOpen);
                    }

                    ImGui::Separator();
                    if (ImGui::MenuItem("Reset Layout")) {
                        for (Panel* panel : panels) {
                            panel->isOpen = true;
                        }
                        result.resetLayout = true;
                    }

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Assets")) {
                    if (ImGui::MenuItem("Show Assets Folder")) {
                        shell::Open(context.paths.assets);
                    }

                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }
        }
        result.hasKeyboard = ImGui::IsWindowFocused();
        ImGui::End();
        ImGui::PopStyleColor(1);

        return result;
    }

    void DrawCreateEntityItems(EditorContext& context) {
        using enum actions::EntityKind;
        for (const actions::EntityKind kind : { eEmpty, eCube, eSphere, ePlane, ePointLight, eSpotLight, eCamera }) {
            if (kind == eCube || kind == ePointLight || kind == eCamera) {
                ImGui::Separator();
            }
            if (ImGui::MenuItem(actions::GetName(kind))) {
                actions::CreateEntity(context, kind);
            }
        }
    }

    void DrawMenuItems(EditorContext& context, std::span<const actions::MenuAction> items, bool isEnabled) {
        for (const actions::MenuAction& action : items) {
            DrawMenuItem(context, action, "", isEnabled);
        }
    }
} // namespace adh
