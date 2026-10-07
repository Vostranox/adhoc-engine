#include "Panel.hpp"
#include <ImGui/imgui.h>

namespace adh {
    Panel::Panel(const char* title, ImGuiWindowFlags windowFlags) noexcept : m_Title{ title }, m_WindowFlags{ windowFlags } {
    }

    void Panel::Draw(EditorContext& context) {
        isFocused = false;
        if (!isOpen) {
            OnHidden(context);
            return;
        }

        if (ImGui::Begin(m_Title, &isOpen, m_WindowFlags)) {
            isFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows | ImGuiFocusedFlags_NoPopupHierarchy);
            OnImGui(context);
        } else {
            OnHidden(context);
        }
        ImGui::End();
    }

    void Panel::OnHidden(EditorContext&) {
    }

    const char* Panel::GetTitle() const noexcept {
        return m_Title;
    }
} // namespace adh
