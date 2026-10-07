#include "SettingsPanel.hpp"
#include "../EditorContext.hpp"
#include "../Widgets.hpp"

#include <Renderer/RenderSettings.hpp>
#include <Renderer/SunShadows.hpp>

#include <ImGui/imgui.h>

#include <iterator>

namespace adh {
    SettingsPanel::SettingsPanel() noexcept : Panel{ "Settings" } {
    }

    void SettingsPanel::OnImGui(EditorContext& context) {
        RenderSettings& renderSettings{ *context.renderSettings };
        if (!widgets::BeginProperties("Settings")) {
            return;
        }

        auto& io{ ImGui::GetIO() };
        widgets::Property("Frame Time");
        ImGui::Text("%.3f ms (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        widgets::Checkbox("Editor FPS Limit", *context.fpsLimit);

        widgets::Property("Render Path");
        const char* renderPaths[]{ "Forward", "Forward+", "Deferred" };
        int renderPath{ static_cast<int>(renderSettings.renderPath) };
        if (ImGui::Combo("##Render Path", &renderPath, renderPaths, static_cast<int>(std::size(renderPaths)))) {
            renderSettings.renderPath = static_cast<RenderPath>(renderPath);
        }

        widgets::SliderFloat("Bloom Strength", renderSettings.bloomStrength, 0.0f, 1.0f);
        widgets::SliderFloat("Bloom Radius", renderSettings.bloomRadius, 0.0f, 0.05f, "%.4f");
        widgets::SliderFloat("Exposure", renderSettings.exposure, 0.0f, 10.0f);
        widgets::SliderFloat("Sun Intensity", renderSettings.sunIntensity, 0.0f, 100.0f);
        widgets::SliderFloat("Sun X", renderSettings.sunPosition.x, -100.0f, 100.0f);
        widgets::SliderFloat("Sun Y", renderSettings.sunPosition.y, -100.0f, 100.0f);
        widgets::SliderFloat("Sun Z", renderSettings.sunPosition.z, -100.0f, 100.0f);
        widgets::SliderInt("Shadow Cascades", renderSettings.shadowCascades, 0, static_cast<int>(SunShadows::maxCascades), ImGuiSliderFlags_AlwaysClamp);
        widgets::SliderFloat("Shadow Distance", renderSettings.shadowDistance, 1.0f, 500.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
        widgets::SliderFloat("Shadow Split", renderSettings.shadowSplitLambda, 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        widgets::SliderFloat("Shadow Softness", renderSettings.shadowSoftness, 0.0f, 0.5f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        widgets::Checkbox("Show Cascades", renderSettings.showShadowCascades);

        widgets::EndProperties();
    }
} // namespace adh
