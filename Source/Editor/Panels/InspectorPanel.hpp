#pragma once
#include "Panel.hpp"

#include <ImGui/imgui.h>
#include <adh/entity.hpp>

namespace adh {
    class InspectorPanel : public Panel {
      public:
        InspectorPanel() noexcept;

      private:
        void OnImGui(EditorContext& context) override;

        void DrawComponents(EditorContext& context, ecs::Entity entity);

        void AddComponent(EditorContext& context, ecs::Entity entity);

      private:
        ecs::Entity m_NewScriptEntity{ ecs::NULL_ENTITY };
        char m_NewScriptName[256]{};
        ImGuiTextFilter m_ComponentFilter;
    };
} // namespace adh
