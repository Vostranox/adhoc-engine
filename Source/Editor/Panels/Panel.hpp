#pragma once
#include <ImGui/imgui.h>

namespace adh {
    struct EditorContext;

    class Panel {
      public:
        Panel(const char* title, ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None) noexcept;

        Panel(const Panel& rhs) = delete;

        Panel& operator=(const Panel& rhs) = delete;

        virtual ~Panel() = default;

        void Draw(EditorContext& context);

        const char* GetTitle() const noexcept;

      private:
        virtual void OnImGui(EditorContext& context) = 0;

        virtual void OnHidden(EditorContext& context);

      public:
        bool isOpen{ true };
        bool isFocused{ false };

      private:
        const char* m_Title;
        ImGuiWindowFlags m_WindowFlags;
    };
} // namespace adh
