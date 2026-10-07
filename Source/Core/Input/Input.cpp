#include "Input.hpp"
#include <Event/Event.hpp>

#if defined(ADH_WINDOWS)
#    include <windows.h>
#    include <xinput.h>
#endif

namespace adh {
    Input::Input() {
    }

    Input::~Input() {
        EventBus().destroy(m_EventSubscriber);
    }

    void Input::Initialize() {
        m_EventSubscriber = EventBus().create_subscriber();
        EventBus().subscribe<KeyboardEvent>(m_EventSubscriber, this, &Input::OnKeyboardEvent);
        EventBus().subscribe<MouseMoveEvent>(m_EventSubscriber, this, &Input::OnMouseMoveEvent);
        EventBus().subscribe<MouseButtonEvent>(m_EventSubscriber, this, &Input::OnMouseButtonEvent);
        EventBus().subscribe<MouseWheelEvent>(m_EventSubscriber, this, &Input::OnMouseWheelEvent);
        EventBus().subscribe<ControllerEvent>(m_EventSubscriber, this, &Input::OnControllerEvent);
        EventBus().subscribe<WindowEvent>(m_EventSubscriber, this, &Input::OnWindowEvent);
    }

    bool Input::GetKey(Keycode keycode) noexcept {
        if (m_Keyboard.GetKeyState(keycode) == KeyboardEvent::Type::eKeyDown ||
            m_Keyboard.GetKeyState(keycode) == KeyboardEvent::Type::eKeyRepeat) {
            return true;
        }

        return false;
    }

    bool Input::RepeatGetKey(Keycode keycode) noexcept {
        if (m_RepeatKeyboard.GetKeyState(keycode) == KeyboardEvent::Type::eKeyDown ||
            m_RepeatKeyboard.GetKeyState(keycode) == KeyboardEvent::Type::eKeyRepeat) {
            return true;
        }

        return false;
    }

    bool Input::GetKeyDown(Keycode keycode) noexcept {
        if (m_Keyboard.GetKeyState(keycode) == KeyboardEvent::Type::eKeyDown) {
            return true;
        }

        return false;
    }

    bool Input::GetKeyUp(Keycode keycode) noexcept {
        if (m_Keyboard.GetKeyState(keycode) == KeyboardEvent::Type::eKeyUp) {
            return true;
        }

        return false;
    }

    Mouse::Position Input::GetMousePosition() const noexcept {
        return m_Mouse.GetPosition();
    }

    float Input::GetMousePositionX() const noexcept {
        return static_cast<float>(m_Mouse.GetPosition().x);
    }

    float Input::GetMousePositionY() const noexcept {
        return static_cast<float>(m_Mouse.GetPosition().y);
    }

    bool Input::GetLeftMouseButtonDown() const noexcept {
        if (m_Mouse.GetButtonState(0) == MouseButtonEvent::Type::eLeftButtonDown) {
            return true;
        }

        return false;
    }

    bool Input::GetLeftMouseButtonUp() const noexcept {
        if (m_Mouse.GetButtonState(0) == MouseButtonEvent::Type::eLeftButtonUp) {
            return true;
        }

        return false;
    }

    bool Input::GetRightMouseButtonDown() const noexcept {
        if (m_Mouse.GetButtonState(1) == MouseButtonEvent::Type::eRightButtonDown) {
            return true;
        }

        return false;
    }

    bool Input::GetRightMouseButtonUp() const noexcept {
        if (m_Mouse.GetButtonState(1) == MouseButtonEvent::Type::eRightButtonUp) {
            return true;
        }

        return false;
    }

    bool Input::GetMiddleMouseButtonDown() const noexcept {
        if (m_Mouse.GetButtonState(2) == MouseButtonEvent::Type::eMiddleButtonDown) {
            return true;
        }

        return false;
    }

    bool Input::GetMiddleMouseButtonUp() const noexcept {
        if (m_Mouse.GetButtonState(2) == MouseButtonEvent::Type::eMiddleButtonUp) {
            return true;
        }

        return false;
    }

    float Input::GetMouseWheelDelta() const noexcept {
        return m_Mouse.GetWheelDelta();
    }

    bool Input::GetButton(Keycode keycode, std::uint32_t controllerId) noexcept {
        if (controllerId >= numberOfControllers) {
            return false;
        }

        if (m_Controller[controllerId].GetButtonState(keycode) == ControllerEvent::Type::eButtonDown ||
            m_Controller[controllerId].GetButtonState(keycode) == ControllerEvent::Type::eButtonRepeat) {
            return true;
        }

        return false;
    }

    bool Input::RepeatGetButton(Keycode keycode, std::uint32_t controllerId) noexcept {
        if (controllerId >= numberOfControllers) {
            return false;
        }

        if (m_RepeatController[controllerId].GetButtonState(keycode) == ControllerEvent::Type::eButtonDown ||
            m_RepeatController[controllerId].GetButtonState(keycode) == ControllerEvent::Type::eButtonRepeat) {
            return true;
        }

        return false;
    }

    bool Input::GetButtonDown(Keycode keycode, std::uint32_t controllerId) noexcept {
        if (controllerId >= numberOfControllers) {
            return false;
        }

        if (m_Controller[controllerId].GetButtonState(keycode) == ControllerEvent::Type::eButtonDown) {
            return true;
        }

        return false;
    }

    bool Input::GetButtonUp(Keycode keycode, std::uint32_t controllerId) noexcept {
        if (controllerId >= numberOfControllers) {
            return false;
        }

        if (m_Controller[controllerId].GetButtonState(keycode) == ControllerEvent::Type::eButtonUp) {
            return true;
        }

        return false;
    }

    void Input::SetControllerVibration([[maybe_unused]] std::uint32_t leftMotorIntensity, [[maybe_unused]] std::uint32_t rightMotorIntensity, [[maybe_unused]] std::uint32_t controllerId) noexcept {
#if defined(ADH_WINDOWS)
        XINPUT_VIBRATION vibration{};
        vibration.wLeftMotorSpeed  = static_cast<WORD>(leftMotorIntensity * 1000u);
        vibration.wRightMotorSpeed = static_cast<WORD>(rightMotorIntensity * 1000u);
        XInputSetState(controllerId, &vibration);

#endif
    }

    std::uint32_t Input::GetControllerBatteryLevel([[maybe_unused]] std::uint32_t controllerId) const noexcept {
#if defined(ADH_WINDOWS)
        XINPUT_BATTERY_INFORMATION batteryInfo{};
        XInputGetBatteryInformation(controllerId, BATTERY_DEVTYPE_GAMEPAD, &batteryInfo);
        return batteryInfo.BatteryLevel;
#else
        return 0u;
#endif
    }

    void Input::PollEvents() noexcept {
        for (std::uint32_t i{}; i != numberOfControllers; ++i) {
#if defined(ADH_WINDOWS)
            XINPUT_STATE state{};
            std::array<bool, ADH_BUTTON_COUNT> pressed{};
            if (m_IsFocused && XInputGetState(i, &state) == ERROR_SUCCESS) {
                constexpr WORD masks[]{ XINPUT_GAMEPAD_A, XINPUT_GAMEPAD_B, XINPUT_GAMEPAD_X, XINPUT_GAMEPAD_Y,
                                        XINPUT_GAMEPAD_BACK, XINPUT_GAMEPAD_START, XINPUT_GAMEPAD_LEFT_THUMB, XINPUT_GAMEPAD_RIGHT_THUMB,
                                        XINPUT_GAMEPAD_LEFT_SHOULDER, XINPUT_GAMEPAD_RIGHT_SHOULDER, XINPUT_GAMEPAD_DPAD_UP, XINPUT_GAMEPAD_DPAD_DOWN,
                                        XINPUT_GAMEPAD_DPAD_LEFT, XINPUT_GAMEPAD_DPAD_RIGHT };
                for (std::size_t button{}; button < std::size(masks); ++button) {
                    pressed[button] = (state.Gamepad.wButtons & masks[button]) != 0;
                }
                pressed[ADH_BUTTON_LTRIGGER] = state.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
                pressed[ADH_BUTTON_RTRIGGER] = state.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
            }
            for (std::size_t button{}; button < pressed.size(); ++button) {
                if (pressed[button]) {
                    EventBus().publish<ControllerEvent>(m_ControllerButtons[i][button] ? ControllerEvent::Type::eButtonRepeat : ControllerEvent::Type::eButtonDown, button, i);
                } else if (m_ControllerButtons[i][button]) {
                    EventBus().publish<ControllerEvent>(ControllerEvent::Type::eButtonUp, button, i);
                }
            }
            m_ControllerButtons[i] = pressed;
#endif
        }
    }

    void Input::OnUpdate() noexcept {
        m_Keyboard.OnUpdate();
        m_Mouse.OnUpdate();
        for (std::uint32_t i{}; i != numberOfControllers; ++i) {
            m_Controller[i].OnUpdate();
        }
    }

    bool Input::OnKeyboardEvent(KeyboardEvent& keyboardEvent) noexcept {
        m_Keyboard.SetKeyState(&keyboardEvent);
        m_RepeatKeyboard.SetKeyState(&keyboardEvent);
        return true;
    }

    bool Input::OnMouseMoveEvent(MouseMoveEvent& mouseEvent) noexcept {
        m_Mouse.SetPosition(&mouseEvent);
        return true;
    }

    bool Input::OnMouseButtonEvent(MouseButtonEvent& mouseEvent) noexcept {
        m_Mouse.SetButtonState(&mouseEvent);
        return true;
    }

    bool Input::OnMouseWheelEvent(MouseWheelEvent& mouseEvent) noexcept {
        m_Mouse.SetWheelDelta(&mouseEvent);
        return true;
    }

    bool Input::OnControllerEvent(ControllerEvent& controllerEvent) noexcept {
        if (controllerEvent.id < numberOfControllers) {
            m_Controller[controllerEvent.id].SetButtonState(&controllerEvent);
            m_RepeatController[controllerEvent.id].SetButtonState(&controllerEvent);
        }
        return true;
    }

    bool Input::OnWindowEvent(WindowEvent& event) noexcept {
        if (event.type == WindowEvent::Type::eFocus) {
            m_IsFocused = true;
        }
        if (event.type == WindowEvent::Type::eKillfocus) {
            m_IsFocused = false;
            m_Mouse.OnKillFocus();
            m_Keyboard.OnKillFocus();
            m_RepeatKeyboard.OnKillFocus();
            for (std::uint32_t i{}; i != numberOfControllers; ++i) {
                m_Controller[i].OnKillFocus();
                m_RepeatController[i].OnKillFocus();
            }
        }
        return false;
    }
} // namespace adh
