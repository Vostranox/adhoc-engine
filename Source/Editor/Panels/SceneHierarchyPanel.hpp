#pragma once
#include "Panel.hpp"

#include <adh/entity.hpp>

namespace adh {
    class SceneHierarchyPanel : public Panel {
      public:
        SceneHierarchyPanel() noexcept;

      private:
        void OnImGui(EditorContext& context) override;

        void OnHidden(EditorContext& context) override;

        void DrawEntity(EditorContext& context, ecs::Entity entity);

        void DrawNameField(EditorContext& context, ecs::Entity entity);

      private:
        ecs::Entity m_NameFieldEntity{ ecs::NULL_ENTITY };
    };
} // namespace adh
