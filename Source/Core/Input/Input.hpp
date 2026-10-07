#pragma once
#include "Controller.hpp"
#include "Keyboard.hpp"
#include "Keycodes.hpp"
#include "Mouse.hpp"
#include <Event/Event.hpp>
#include <array>

namespace adh {
    class Input {
      public:
        static constexpr std::uint32_t numberOfControllers{ 4u };

      public:
        Input();

        ~Input();

        void Initialize();

        bool GetKey(Keycode keycode) noexcept;

        bool RepeatGetKey(Keycode keycode) noexcept;

        bool GetKeyDown(Keycode keycode) noexcept;

        bool GetKeyUp(Keycode keycode) noexcept;

        Mouse::Position GetMousePosition() const noexcept;

        float GetMousePositionX() const noexcept;

        float GetMousePositionY() const noexcept;

        bool GetLeftMouseButtonDown() const noexcept;

        bool GetLeftMouseButtonUp() const noexcept;

        bool GetRightMouseButtonDown() const noexcept;

        bool GetRightMouseButtonUp() const noexcept;

        bool GetMiddleMouseButtonDown() const noexcept;

        bool GetMiddleMouseButtonUp() const noexcept;

        float GetMouseWheelDelta() const noexcept;

        bool GetButton(Keycode keycode, std::uint32_t controllerId = 0u) noexcept;

        bool RepeatGetButton(Keycode keycode, std::uint32_t controllerId = 0u) noexcept;

        bool GetButtonDown(Keycode keycode, std::uint32_t controllerId = 0u) noexcept;

        bool GetButtonUp(Keycode keycode, std::uint32_t controllerId = 0u) noexcept;

        void SetControllerVibration(std::uint32_t leftMotorIntensity, std::uint32_t rightMotorIntensity, std::uint32_t controllerId = 0u) noexcept;

        std::uint32_t GetControllerBatteryLevel(std::uint32_t controllerId = 0u) const noexcept;

        void PollEvents() noexcept;

        void OnUpdate() noexcept;

      private:
        bool OnKeyboardEvent(KeyboardEvent& keyboardEvent) noexcept;

        bool OnMouseMoveEvent(MouseMoveEvent& mouseEvent) noexcept;

        bool OnMouseButtonEvent(MouseButtonEvent& mouseEvent) noexcept;

        bool OnMouseWheelEvent(MouseWheelEvent& mouseEvent) noexcept;

        bool OnControllerEvent(ControllerEvent& controllerEvent) noexcept;

        bool OnWindowEvent(WindowEvent& event) noexcept;

      private:
        bool m_IsFocused{ true };
#if defined(ADH_WINDOWS)
        std::array<std::array<bool, ADH_BUTTON_COUNT>, numberOfControllers> m_ControllerButtons{};
#endif
        Keyboard m_Keyboard;
        Keyboard m_RepeatKeyboard;
        Mouse m_Mouse;
        Controller m_Controller[numberOfControllers];
        Controller m_RepeatController[numberOfControllers];
        event::Subscriber m_EventSubscriber{ event::NULL_SUBSCRIBER };
    };
} // namespace adh
