#pragma once
#include "Panel.hpp"

namespace adh {
    class SettingsPanel : public Panel {
      public:
        SettingsPanel() noexcept;

      private:
        void OnImGui(EditorContext& context) override;
    };
} // namespace adh
