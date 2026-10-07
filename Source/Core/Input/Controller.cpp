#include "Controller.hpp"

namespace adh {
    Controller::Controller() noexcept {
        Flush();
    }

    void Controller::SetButtonState(ControllerEvent* event) noexcept {
        if (event->keycode < m_ButtonStates.GetSize()) {
            m_ButtonStates[event->keycode] = event->type;
        }
    }

    Controller::State Controller::GetButtonState(std::uint64_t keycode) const noexcept {
        return keycode < m_ButtonStates.GetSize() ? m_ButtonStates[keycode] : State::eInvalid;
    }

    void Controller::OnUpdate() noexcept {
        Flush();
    }

    void Controller::OnKillFocus() noexcept {
        Flush();
    }

    void Controller::Flush() noexcept {
        m_ButtonStates.Fill(State::eInvalid);
    }
} // namespace adh
