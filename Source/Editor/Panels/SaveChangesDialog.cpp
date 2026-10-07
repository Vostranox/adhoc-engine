#include "SaveChangesDialog.hpp"
#include "../EditorActions.hpp"
#include "../EditorContext.hpp"

#include <Utf8.hpp>

#include <ImGui/imgui.h>

#include <functional>
#include <string>
#include <utility>

namespace adh {
    void DrawSaveChangesDialog(EditorContext& context) {
        constexpr const char* title{ "Save Changes?" };
        if (context.actionAfterSaveQuestion && !ImGui::IsPopupOpen(title)) {
            ImGui::OpenPopup(title);
        }

        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        if (!ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) {
            return;
        }
        const std::string name{ context.scenePath.empty() ? std::string{ "Untitled" } : ToUtf8(context.scenePath.filename()) };
        ImGui::Text("Save the changes to %s?", name.c_str());
        ImGui::TextUnformatted("They are lost if you don't save them.");
        ImGui::Spacing();

        const ImVec2 buttonSize{ 120.0f, 0.0f };
        if (ImGui::Button("Save", buttonSize)) {
            actions::SaveSceneThen(context, std::exchange(context.actionAfterSaveQuestion, {}));
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Don't Save", buttonSize)) {
            const std::function<void()> action{ std::exchange(context.actionAfterSaveQuestion, {}) };
            action();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", buttonSize) || ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
            context.actionAfterSaveQuestion = {};
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
} // namespace adh
