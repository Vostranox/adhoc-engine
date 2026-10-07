#include "GamePanel.hpp"
#include "../EditorContext.hpp"
#include <ImGui/imgui.h>

#include <iterator>

namespace adh {
    static void DrawAspectRatioCombo(GameViewAspect& aspect) {
        struct Preset {
            const char* name;
            float width;
            float height;
        };
        static constexpr Preset presets[]{
            { "Screen Aspect", 0.0f, 0.0f },
            { "16/9 Aspect", 16.0f, 9.0f },
            { "16/10 Aspect", 16.0f, 10.0f },
            { "3/2 Aspect", 3.0f, 2.0f },
            { "4/3 Aspect", 4.0f, 3.0f },
            { "9/16 Aspect", 9.0f, 16.0f },
        };

        if (ImGui::BeginCombo("##Aspect Ratio", presets[aspect.preset].name, ImGuiComboFlags_WidthFitPreview)) {
            for (int i{}; i != static_cast<int>(std::size(presets)); ++i) {
                if (ImGui::Selectable(presets[i].name, i == aspect.preset)) {
                    const Preset& preset{ presets[i] };
                    aspect.preset = i;
                    aspect.width  = preset.width > 0.0f ? preset.width : aspect.screenWidth;
                    aspect.height = preset.height > 0.0f ? preset.height : aspect.screenHeight;
                }
            }
            ImGui::EndCombo();
        }
    }

    GamePanel::GamePanel() noexcept : ViewportPanel{ "Game", ImGuiWindowFlags_NoNavInputs } {
    }

    void GamePanel::OnImGui(EditorContext& context) {
        DrawAspectRatioCombo(context.gameAspect);
        DrawImage(context.gameAspect.GetRatio());
    }
} // namespace adh
