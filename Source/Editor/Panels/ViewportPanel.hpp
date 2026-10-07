#pragma once
#include "Panel.hpp"
#include "ViewportRect.hpp"

#include <ImGui/imgui.h>

#include <optional>

namespace adh {
    class ViewportPanel : public Panel {
      public:
        ViewportPanel(const char* title, ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None) noexcept;

      protected:
        ImVec2 DrawImage(std::optional<float> aspectRatio = std::nullopt);

        void OnHidden(EditorContext& context) override;

      public:
        void* texture{};
        ViewportRect rect;
        ViewportImage image;
    };
} // namespace adh
