#include "Toolbar.hpp"
#include "../EditorActions.hpp"
#include "../EditorContext.hpp"
#include "../IconFontCppHeaders/IconFontAwesome5.hpp"
#include "../Theme.hpp"
#include "../Widgets.hpp"

#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>

#include <algorithm>
#include <string>

namespace adh {
    static float IconButtonWidth() {
        return ImGui::GetFrameHeight() + 2.0f * ImGui::GetStyle().FramePadding.x;
    }

    static bool IconButton(const char* icon, const char* tooltip, bool isOn, bool isEnabled = true) {
        return widgets::ToggleButton(icon, tooltip, isOn, isEnabled, IconButtonWidth());
    }

    static std::string GetTooltip(const actions::MenuAction& action) {
        return std::string{ action.label } + " (" + ImGui::GetKeyChordName(action.shortcut) + ")";
    }

    static void Gap() {
        ImGui::Dummy(ImVec2{ ImGui::GetFrameHeight(), 0.0f });
    }

    static void DrawGuizmoButtons(EditorContext& context) {
        GuizmoSettings& guizmo{ context.guizmo };
        if (IconButton(ICON_FA_ARROWS_ALT, GetTooltip(actions::translateAction).c_str(), guizmo.mode == GuizmoMode::eTranslate)) {
            actions::translateAction.run(context);
        }
        if (IconButton(ICON_FA_SYNC_ALT, GetTooltip(actions::rotateAction).c_str(), guizmo.mode == GuizmoMode::eRotate)) {
            actions::rotateAction.run(context);
        }
        if (IconButton(ICON_FA_EXPAND_ARROWS_ALT, GetTooltip(actions::scaleAction).c_str(), guizmo.mode == GuizmoMode::eScale)) {
            actions::scaleAction.run(context);
        }
        Gap();

        constexpr const char* local{ ICON_FA_CUBE " Local###Axes" };
        constexpr const char* world{ ICON_FA_GLOBE " World###Axes" };
        const float axesWidth{ std::max(ImGui::CalcTextSize(local, nullptr, true).x, ImGui::CalcTextSize(world, nullptr, true).x) +
                               2.0f * ImGui::GetStyle().FramePadding.x };
        if (widgets::ToggleButton(guizmo.isLocal ? local : world, GetTooltip(actions::guizmoAxesAction).c_str(), false, true, axesWidth)) {
            actions::guizmoAxesAction.run(context);
        }
        Gap();

        if (IconButton(ICON_FA_MAGNET, "Snap", guizmo.snaps)) {
            guizmo.snaps = !guizmo.snaps;
        }

        float* step{};
        float speed{};
        float min{};
        float max{};
        const char* format{};
        const char* tooltip{};
        switch (guizmo.mode) {
        case GuizmoMode::eTranslate:
            step    = &guizmo.translateStep;
            speed   = 0.01f;
            min     = 0.01f;
            max     = 100.0f;
            format  = "%.2f";
            tooltip = "Translate snaps to this step";
            break;
        case GuizmoMode::eRotate:
            step    = &guizmo.rotateStep;
            speed   = 0.5f;
            min     = 1.0f;
            max     = 180.0f;
            format  = "%.0f\xC2\xB0";
            tooltip = "Rotate snaps to this angle";
            break;
        case GuizmoMode::eScale:
            step    = &guizmo.scaleStep;
            speed   = 0.01f;
            min     = 0.01f;
            max     = 10.0f;
            format  = "%.2f";
            tooltip = "Scale snaps to this step";
            break;
        }
        ImGui::SetNextItemWidth(2.0f * IconButtonWidth());
        ImGui::DragFloat("##Snap Step", step, speed, min, max, format, ImGuiSliderFlags_AlwaysClamp);
        ImGui::SetItemTooltip("%s", tooltip);
    }

    static void DrawPlayButtons(EditorContext& context) {
        const float width{ 3.0f * IconButtonWidth() + 2.0f * ImGui::GetStyle().ItemSpacing.x };
        ImGui::SetCursorPosX(std::max(ImGui::GetCursorPosX(), (ImGui::GetWindowWidth() - width) / 2.0f));

        if (IconButton(ICON_FA_PLAY, GetTooltip(actions::playAction).c_str(), context.isPlaying)) {
            actions::playAction.run(context);
        }
        if (IconButton(ICON_FA_PAUSE, context.isPaused ? "Unpause" : "Pause", context.isPaused, context.isPlaying)) {
            if (context.isPaused) {
                actions::Unpause(context);
            } else {
                actions::Pause(context);
            }
        }
        if (IconButton(ICON_FA_STOP, "Stop", false, context.isPlaying)) {
            actions::Stop(context);
        }
    }

    static void DrawPlayOptions(EditorContext& context) {
        constexpr const char* label{ ICON_FA_EXPAND " Maximize on Play" };
        const float width{ ImGui::CalcTextSize(label).x + 2.0f * ImGui::GetStyle().FramePadding.x };
        ImGui::SetCursorPosX(std::max(ImGui::GetCursorPosX(), ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x - width));

        bool& maximizeOnPlay{ *context.maximizeOnPlay };
        if (widgets::ToggleButton(label, "Play in the whole window: Escape brings the editor back", maximizeOnPlay)) {
            maximizeOnPlay = !maximizeOnPlay;
        }
    }

    void DrawToolbar(EditorContext& context) {
        if (!ImGui::BeginMenuBar()) {
            return;
        }
        const ImGuiWindow* window{ ImGui::GetCurrentWindow() };
        const ImRect bar{ window->MenuBarRect() };
        window->DrawList->AddRectFilled(bar.Min, bar.Max, ImGui::GetColorU32(context.isPlaying ? theme::playingBar : theme::bar));
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (bar.GetHeight() - ImGui::GetFrameHeight()) / 2.0f);

        ImGui::PushItemFlag(ImGuiItemFlags_NoFocus, true);
        DrawGuizmoButtons(context);
        DrawPlayButtons(context);
        DrawPlayOptions(context);
        ImGui::PopItemFlag();
        ImGui::EndMenuBar();
    }
} // namespace adh
