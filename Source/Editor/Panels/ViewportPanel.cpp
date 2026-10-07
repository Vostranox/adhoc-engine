#include "ViewportPanel.hpp"

#include <cmath>

namespace adh {
    static ImVec2 GetLargestViewportSize(std::optional<float> aspectRatio) {
        ImVec2 ret{ ImGui::GetContentRegionAvail() };
        ret.x -= ImGui::GetScrollX();
        ret.y -= ImGui::GetScrollY();

        if (aspectRatio) {
            float aspectWidth{ ret.x };
            float aspectHeight{ aspectWidth / *aspectRatio };

            if (aspectHeight > ret.y) {
                aspectHeight = ret.y;
                aspectWidth  = aspectHeight * *aspectRatio;
            }
            ret = ImVec2{ aspectWidth, aspectHeight };
        }

        return ImVec2{ std::floor(ret.x), std::floor(ret.y) };
    }

    static ImVec2 GetCenterPos(ImVec2 aspectSize) {
        ImVec2 ret{ ImGui::GetContentRegionAvail() };
        ret.x -= ImGui::GetScrollX();
        ret.y -= ImGui::GetScrollY();

        float viewportX{ (ret.x / 2.0f) - (aspectSize.x / 2.0f) };
        float viewportY{ (ret.y / 2.0f) - (aspectSize.y / 2.0f) };

        return ImVec2{ std::floor(viewportX + ImGui::GetCursorPosX()), std::floor(viewportY + ImGui::GetCursorPosY()) };
    }

    ViewportPanel::ViewportPanel(const char* title, ImGuiWindowFlags windowFlags) noexcept : Panel{ title, windowFlags } {
    }

    ImVec2 ViewportPanel::DrawImage(std::optional<float> aspectRatio) {
        const ImVec2 size{ GetLargestViewportSize(aspectRatio) };
        ImGui::SetCursorPos(GetCenterPos(size));

        auto cursorScreenPos{ ImGui::GetCursorScreenPos() };
        cursorScreenPos.x -= ImGui::GetScrollX();
        cursorScreenPos.y -= ImGui::GetScrollY();

        rect.left   = cursorScreenPos.x;
        rect.top    = cursorScreenPos.y;
        rect.right  = cursorScreenPos.x + size.x;
        rect.bottom = cursorScreenPos.y + size.y;

        ImGui::Image(reinterpret_cast<ImTextureID>(texture), size, ImVec2(0, 1), ImVec2(1, 0));
        const ImVec2 scale{ ImGui::GetIO().DisplayFramebufferScale };
        image = ViewportImage{ true, size.x * scale.x, size.y * scale.y };
        return size;
    }

    void ViewportPanel::OnHidden(EditorContext&) {
        rect  = {};
        image = {};
    }
} // namespace adh
