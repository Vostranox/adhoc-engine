#pragma once
#include "ViewportPanel.hpp"

namespace adh {
    class GamePanel : public ViewportPanel {
      public:
        GamePanel() noexcept;

      private:
        void OnImGui(EditorContext& context) override;
    };
} // namespace adh
