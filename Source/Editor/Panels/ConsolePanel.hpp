#pragma once
#include "Panel.hpp"

#include <Event/Event.hpp>

#include <ImGui/imgui.h>

#include <deque>
#include <string>

namespace adh {
    struct ConsoleLine {
        EditorLogEvent::Type type;
        bool isFirst;
        std::string time;
        std::string text;
    };

    class ConsolePanel : public Panel {
      public:
        ConsolePanel();

        ~ConsolePanel();

        bool OnLogEvent(EditorLogEvent& event) noexcept;

      private:
        void OnImGui(EditorContext& context) override;

      private:
        static constexpr std::size_t maxLines{ 5'000 };

        std::deque<ConsoleLine> m_Lines;
        bool m_ShowsInfo{ true };
        bool m_ShowsErrors{ true };
        ImGuiTextFilter m_Search;

        event::Subscriber listener{ event::NULL_SUBSCRIBER };
    };
} // namespace adh
