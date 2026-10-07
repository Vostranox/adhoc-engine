#include "Keyboard.hpp"

namespace adh {
    Keyboard::Keyboard() noexcept {
        Flush();
    }

    void Keyboard::SetKeyState(KeyboardEvent* event) noexcept {
        if (event->keycode < m_KeyStates.GetSize()) {
            m_KeyStates[event->keycode] = event->type;
        }
    }

    Keyboard::State Keyboard::GetKeyState(std::uint64_t keycode) const noexcept {
        return keycode < m_KeyStates.GetSize() ? m_KeyStates[keycode] : State::eInvalid;
    }

    void Keyboard::OnUpdate() noexcept {
        Flush();
    }

    void Keyboard::OnKillFocus() noexcept {
        Flush();
    }

    void Keyboard::Flush() noexcept {
        m_KeyStates.Fill(State::eInvalid);
    }
} // namespace adh
