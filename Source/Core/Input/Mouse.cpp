#include "Mouse.hpp"

namespace adh {
    Mouse::Mouse() noexcept {
        Flush();
    }

    void Mouse::SetPosition(MouseMoveEvent* event) noexcept {
        m_Pos.x = event->x;
        m_Pos.y = event->y;
    }

    void Mouse::SetButtonState(MouseButtonEvent* event) noexcept {
        SetPosition(event);
        m_ButtonStates[static_cast<std::int32_t>(event->index)] = event->type;
    }

    void Mouse::SetWheelDelta(MouseWheelEvent* event) noexcept {
        m_WheelDelta += event->delta;
    }

    Mouse::Position Mouse::GetPosition() const noexcept {
        return m_Pos;
    }

    Mouse::State Mouse::GetButtonState(std::uint32_t index) const noexcept {
        return m_ButtonStates[index];
    }

    float Mouse::GetWheelDelta() const noexcept {
        return m_WheelDelta;
    }

    void Mouse::OnUpdate() noexcept {
        Flush();
    }

    void Mouse::OnKillFocus() noexcept {
        Flush();
    }

    void Mouse::Flush() noexcept {
        m_ButtonStates.Fill(State::eInvalid);
        m_WheelDelta = 0.0f;
    }
} // namespace adh
