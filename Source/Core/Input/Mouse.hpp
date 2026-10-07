#pragma once
#include <Event/EventTypes.hpp>
#include <Std/StaticArray.hpp>

namespace adh {
    class Mouse {
        friend class Input;

      public:
        struct Position {
            std::int32_t x;
            std::int32_t y;
        };

      public:
        using State = MouseButtonEvent::Type;

      private:
        Mouse() noexcept;

        void SetPosition(MouseMoveEvent* event) noexcept;

        void SetButtonState(MouseButtonEvent* event) noexcept;

        void SetWheelDelta(MouseWheelEvent* event) noexcept;

        Position GetPosition() const noexcept;

        State GetButtonState(std::uint32_t index) const noexcept;

        float GetWheelDelta() const noexcept;

        void OnUpdate() noexcept;

        void OnKillFocus() noexcept;

        void Flush() noexcept;

      private:
        Position m_Pos{};
        float m_WheelDelta{};
        StaticArray<State, 3u> m_ButtonStates;
    };
} // namespace adh
