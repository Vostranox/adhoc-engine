#pragma once
#include <Event/EventTypes.hpp>

#include <array>

namespace adh {
    class EditorInput {
      public:
        static void NewFrame();

        static bool HasQueuedEvents();

        void OnKeyboard(const KeyboardEvent& event);

        void OnCharacter(const CharEvent& event);

        void OnMouseMove(const MouseMoveEvent& event);

        void OnMouseButton(const MouseButtonEvent& event);

        void OnMouseWheel(const MouseWheelEvent& event);

        void OnFocus(bool focused);

        void Reset();

      private:
        std::array<bool, 8> m_Modifiers{};
    };
} // namespace adh
