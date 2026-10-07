#include "EditorInput.hpp"

#include <Input/Keycodes.hpp>

#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>

#include <utility>

namespace adh {
    static ImGuiKey ToImGuiKey(std::uint64_t keycode) {
        static constexpr std::pair<std::uint64_t, ImGuiKey> keys[]{
            { ADH_KEY_A, ImGuiKey_A },
            { ADH_KEY_B, ImGuiKey_B },
            { ADH_KEY_C, ImGuiKey_C },
            { ADH_KEY_D, ImGuiKey_D },
            { ADH_KEY_E, ImGuiKey_E },
            { ADH_KEY_F, ImGuiKey_F },
            { ADH_KEY_G, ImGuiKey_G },
            { ADH_KEY_H, ImGuiKey_H },
            { ADH_KEY_I, ImGuiKey_I },
            { ADH_KEY_J, ImGuiKey_J },
            { ADH_KEY_K, ImGuiKey_K },
            { ADH_KEY_L, ImGuiKey_L },
            { ADH_KEY_M, ImGuiKey_M },
            { ADH_KEY_N, ImGuiKey_N },
            { ADH_KEY_O, ImGuiKey_O },
            { ADH_KEY_P, ImGuiKey_P },
            { ADH_KEY_Q, ImGuiKey_Q },
            { ADH_KEY_R, ImGuiKey_R },
            { ADH_KEY_S, ImGuiKey_S },
            { ADH_KEY_T, ImGuiKey_T },
            { ADH_KEY_U, ImGuiKey_U },
            { ADH_KEY_V, ImGuiKey_V },
            { ADH_KEY_W, ImGuiKey_W },
            { ADH_KEY_X, ImGuiKey_X },
            { ADH_KEY_Y, ImGuiKey_Y },
            { ADH_KEY_Z, ImGuiKey_Z },
            { ADH_KEY_0, ImGuiKey_0 },
            { ADH_KEY_1, ImGuiKey_1 },
            { ADH_KEY_2, ImGuiKey_2 },
            { ADH_KEY_3, ImGuiKey_3 },
            { ADH_KEY_4, ImGuiKey_4 },
            { ADH_KEY_5, ImGuiKey_5 },
            { ADH_KEY_6, ImGuiKey_6 },
            { ADH_KEY_7, ImGuiKey_7 },
            { ADH_KEY_8, ImGuiKey_8 },
            { ADH_KEY_9, ImGuiKey_9 },
            { ADH_F1, ImGuiKey_F1 },
            { ADH_F2, ImGuiKey_F2 },
            { ADH_F3, ImGuiKey_F3 },
            { ADH_F4, ImGuiKey_F4 },
            { ADH_F5, ImGuiKey_F5 },
            { ADH_F6, ImGuiKey_F6 },
            { ADH_F7, ImGuiKey_F7 },
            { ADH_F8, ImGuiKey_F8 },
            { ADH_F9, ImGuiKey_F9 },
            { ADH_F10, ImGuiKey_F10 },
            { ADH_F11, ImGuiKey_F11 },
            { ADH_F12, ImGuiKey_F12 },
            { ADH_F13, ImGuiKey_F13 },
            { ADH_F14, ImGuiKey_F14 },
            { ADH_F15, ImGuiKey_F15 },
            { ADH_F16, ImGuiKey_F16 },
            { ADH_F17, ImGuiKey_F17 },
            { ADH_F18, ImGuiKey_F18 },
            { ADH_F19, ImGuiKey_F19 },
            { ADH_F20, ImGuiKey_F20 },
            { ADH_F21, ImGuiKey_F21 },
            { ADH_F22, ImGuiKey_F22 },
            { ADH_F23, ImGuiKey_F23 },
            { ADH_F24, ImGuiKey_F24 },
            { ADH_APOSTROPHE, ImGuiKey_Apostrophe },
            { ADH_COMMA, ImGuiKey_Comma },
            { ADH_MINUS, ImGuiKey_Minus },
            { ADH_PERIOD, ImGuiKey_Period },
            { ADH_SLASH, ImGuiKey_Slash },
            { ADH_SEMICOLON, ImGuiKey_Semicolon },
            { ADH_EQUAL, ImGuiKey_Equal },
            { ADH_LBRACKET, ImGuiKey_LeftBracket },
            { ADH_BACKSLASH, ImGuiKey_Backslash },
            { ADH_RBRACKET, ImGuiKey_RightBracket },
            { ADH_GRAVE, ImGuiKey_GraveAccent },
            { ADH_NUMLOCK, ImGuiKey_NumLock },
            { ADH_SCROLLLOCK, ImGuiKey_ScrollLock },
            { ADH_NUMPAD_ENTER, ImGuiKey_KeypadEnter },
            { ADH_SPACE, ImGuiKey_Space },
            { ADH_RETURN, ImGuiKey_Enter },
            { ADH_TAB, ImGuiKey_Tab },
            { ADH_ESCAPE, ImGuiKey_Escape },
            { ADH_BACK, ImGuiKey_Backspace },
            { ADH_INSERT, ImGuiKey_Insert },
            { ADH_DELETE, ImGuiKey_Delete },
            { ADH_HOME, ImGuiKey_Home },
            { ADH_END, ImGuiKey_End },
            { ADH_PRIOR, ImGuiKey_PageUp },
            { ADH_NEXT, ImGuiKey_PageDown },
            { ADH_LEFT, ImGuiKey_LeftArrow },
            { ADH_RIGHT, ImGuiKey_RightArrow },
            { ADH_UP, ImGuiKey_UpArrow },
            { ADH_DOWN, ImGuiKey_DownArrow },
            { ADH_PAUSE, ImGuiKey_Pause },
            { ADH_CAPITAL, ImGuiKey_CapsLock },
            { ADH_SNAPSHOT, ImGuiKey_PrintScreen },
            { ADH_APPS, ImGuiKey_Menu },
            { ADH_CONTROL, ImGuiKey_LeftCtrl },
            { ADH_RCONTROL, ImGuiKey_RightCtrl },
            { ADH_SHIFT, ImGuiKey_LeftShift },
            { ADH_RSHIFT, ImGuiKey_RightShift },
            { ADH_MENU, ImGuiKey_LeftAlt },
            { ADH_RMENU, ImGuiKey_RightAlt },
            { ADH_SUPER, ImGuiKey_LeftSuper },
            { ADH_RSUPER, ImGuiKey_RightSuper },
            { ADH_MULTIPLY, ImGuiKey_KeypadMultiply },
            { ADH_ADD, ImGuiKey_KeypadAdd },
            { ADH_SUBTRACT, ImGuiKey_KeypadSubtract },
            { ADH_DECIMAL, ImGuiKey_KeypadDecimal },
            { ADH_DIVIDE, ImGuiKey_KeypadDivide },
            { ADH_NUMPAD0, ImGuiKey_Keypad0 },
            { ADH_NUMPAD1, ImGuiKey_Keypad1 },
            { ADH_NUMPAD2, ImGuiKey_Keypad2 },
            { ADH_NUMPAD3, ImGuiKey_Keypad3 },
            { ADH_NUMPAD4, ImGuiKey_Keypad4 },
            { ADH_NUMPAD5, ImGuiKey_Keypad5 },
            { ADH_NUMPAD6, ImGuiKey_Keypad6 },
            { ADH_NUMPAD7, ImGuiKey_Keypad7 },
            { ADH_NUMPAD8, ImGuiKey_Keypad8 },
            { ADH_NUMPAD9, ImGuiKey_Keypad9 },
        };
        for (const auto& [native, imgui] : keys) {
            if (keycode == native) {
                return imgui;
            }
        }
        return ImGuiKey_None;
    }

    void EditorInput::NewFrame() {
        auto& gui{ *GImGui };
        ImVector<ImGuiInputEvent> later;
        if (gui.IO.ConfigInputTrickleEventQueue && gui.IO.WantTextInput) {
            bool textBefore{};
            for (int i{}; i < gui.InputEventsQueue.Size; ++i) {
                const auto type{ gui.InputEventsQueue[i].Type };
                textBefore |= type == ImGuiInputEventType_Text;
                if (textBefore && type == ImGuiInputEventType_MouseButton) {
                    for (int j{ i }; j < gui.InputEventsQueue.Size; ++j) {
                        later.push_back(gui.InputEventsQueue[j]);
                    }
                    gui.InputEventsQueue.resize(i);
                    break;
                }
            }
        }
        ImGui::NewFrame();
        for (const auto& event : later) {
            gui.InputEventsQueue.push_back(event);
        }
    }

    bool EditorInput::HasQueuedEvents() {
        return !GImGui->InputEventsQueue.empty();
    }

    void EditorInput::OnKeyboard(const KeyboardEvent& event) {
        const bool down{ event.type == KeyboardEvent::Type::eKeyDown || event.type == KeyboardEvent::Type::eKeyRepeat };
        if (!down && event.type != KeyboardEvent::Type::eKeyUp) {
            return;
        }
        ImGuiIO& io{ ImGui::GetIO() };
        constexpr std::uint64_t modifiers[]{ ADH_CONTROL, ADH_RCONTROL, ADH_SHIFT, ADH_RSHIFT, ADH_MENU, ADH_RMENU, ADH_SUPER, ADH_RSUPER };
        for (std::size_t i{}; i != m_Modifiers.size(); ++i) {
            if (event.keycode == modifiers[i]) {
                m_Modifiers[i] = down;
            }
        }
        io.AddKeyEvent(ImGuiMod_Ctrl, m_Modifiers[0] || m_Modifiers[1]);
        io.AddKeyEvent(ImGuiMod_Shift, m_Modifiers[2] || m_Modifiers[3]);
        io.AddKeyEvent(ImGuiMod_Alt, m_Modifiers[4] || m_Modifiers[5]);
        io.AddKeyEvent(ImGuiMod_Super, m_Modifiers[6] || m_Modifiers[7]);
        if (const ImGuiKey key{ ToImGuiKey(event.keycode) }; key != ImGuiKey_None) {
            io.AddKeyEvent(key, down);
        }
    }

    void EditorInput::OnCharacter(const CharEvent& event) {
        ImGui::GetIO().AddInputCharacter(event.keycode);
    }

    void EditorInput::OnMouseMove(const MouseMoveEvent& event) {
        ImGui::GetIO().AddMousePosEvent(static_cast<float>(event.x), static_cast<float>(event.y));
    }

    void EditorInput::OnMouseButton(const MouseButtonEvent& event) {
        OnMouseMove(event);
        const bool down{ event.type == MouseButtonEvent::Type::eLeftButtonDown ||
                         event.type == MouseButtonEvent::Type::eRightButtonDown ||
                         event.type == MouseButtonEvent::Type::eMiddleButtonDown };
        const int button{ event.index == MouseButtonEvent::Index::eLeftButton ? 0 : event.index == MouseButtonEvent::Index::eRightButton ? 1
                                                                                                                                         : 2 };
        ImGui::GetIO().AddMouseButtonEvent(button, down);
    }

    void EditorInput::OnMouseWheel(const MouseWheelEvent& event) {
        OnMouseMove(event);
        ImGui::GetIO().AddMouseWheelEvent(0.0f, event.delta);
    }

    void EditorInput::OnFocus(bool focused) {
        if (!focused) {
            m_Modifiers.fill(false);
        }
        ImGui::GetIO().AddFocusEvent(focused);
    }

    void EditorInput::Reset() {
        m_Modifiers.fill(false);
        ImGuiIO& io{ ImGui::GetIO() };
        io.ClearEventsQueue();
        io.ClearInputKeys();
        io.ClearInputMouse();
    }
} // namespace adh
