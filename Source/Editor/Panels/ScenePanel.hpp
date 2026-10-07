#pragma once
#include "../EditorCamera.hpp"
#include "ViewportPanel.hpp"

#include <Scene/Components/Transform.hpp>

#include <adh/entity.hpp>

#include <optional>

namespace adh {
    struct ScenePick {
        float u{};
        float v{};
    };

    class ScenePanel : public ViewportPanel {
      public:
        ScenePanel() noexcept;

        bool IsMovingCamera() const noexcept;

      private:
        struct GuizmoDrag {
            ecs::Entity entity;
            Transform start;
        };

      private:
        void OnImGui(EditorContext& context) override;

        void OnHidden(EditorContext& context) override;

        void CommitGuizmoDrag(EditorContext& context);

      public:
        std::optional<ScenePick> pick;

      private:
        EditorCamera m_Camera;
        std::optional<GuizmoDrag> m_GuizmoDrag;
    };
} // namespace adh
