#pragma once
#include "Keycodes.hpp"
#include <Event/EventTypes.hpp>
#include <Std/StaticArray.hpp>

namespace adh {
    class Keyboard {
        friend class Input;
        friend class Editor;

      public:
        using State = KeyboardEvent::Type;

      private:
        Keyboard() noexcept;

        void SetKeyState(KeyboardEvent* event) noexcept;

        State GetKeyState(std::uint64_t keycode) const noexcept;

        void OnUpdate() noexcept;

        void OnKillFocus() noexcept;

        void Flush() noexcept;

      private:
        StaticArray<State, ADH_KEY_COUNT> m_KeyStates;
    };
} // namespace adh
