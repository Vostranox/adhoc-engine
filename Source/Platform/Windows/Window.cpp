#include <Window.hpp>

#include <Event/Event.hpp>
#include <Input/Keycodes.hpp>

#include <shellapi.h>
#include <windowsx.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>

namespace adh {
    static std::wstring Wide(const char* text) {
        const int count{ MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, -1, nullptr, 0) };
        if (count == 0) {
            return {};
        }
        std::wstring value(static_cast<std::size_t>(count), L'\0');
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, -1, value.data(), count);
        value.pop_back();
        return value;
    }

    static std::string Utf8(const wchar_t* text) {
        const int count{ WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr) };
        if (count == 0) {
            return {};
        }
        std::string value(static_cast<std::size_t>(count), '\0');
        WideCharToMultiByte(CP_UTF8, 0, text, -1, value.data(), count, nullptr, nullptr);
        value.pop_back();
        return value;
    }

    static void PublishCharacter(std::uint32_t value) {
        if (value >= 32 && value != 127 && !(value >= 0xD800 && value <= 0xDFFF)) {
            EventBus().publish<CharEvent>(value);
        }
    }

    Window::~Window() {
        Clear();
    }

    HINSTANCE Window::GetInstance() noexcept {
        return GetModuleHandleW(nullptr);
    }

    HWND Window::GetHandle() const noexcept {
        return m_WindowHandle;
    }

    void Window::Create(const char* name, std::int32_t width, std::int32_t height, bool isPrepared, bool) {
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        constexpr wchar_t className[]{ L"AdHocNativeWindow" };
        WNDCLASSEXW windowClass{};
        windowClass.cbSize        = sizeof(windowClass);
        windowClass.style         = CS_HREDRAW | CS_VREDRAW;
        windowClass.lpfnWndProc   = WindowProcedure;
        windowClass.hInstance     = GetInstance();
        windowClass.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
        windowClass.hIcon         = LoadIconW(nullptr, IDI_APPLICATION);
        windowClass.lpszClassName = className;
        if (!RegisterClassExW(&windowClass)) {
            std::fputs("AdHoc: window class registration failed\n", stderr);
            std::exit(EXIT_FAILURE);
        }
        RECT bounds{ 0, 0, width, height };
        AdjustWindowRectEx(&bounds, WS_OVERLAPPEDWINDOW, FALSE, 0);
        const auto title{ Wide(name) };
        m_Cursor       = LoadCursorW(nullptr, IDC_ARROW);
        m_WindowHandle = CreateWindowExW(0, className, title.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, bounds.right - bounds.left, bounds.bottom - bounds.top, nullptr, nullptr, GetInstance(), this);
        if (!m_WindowHandle) {
            std::fputs("AdHoc: window creation failed\n", stderr);
            std::exit(EXIT_FAILURE);
        }
        RECT client{};
        GetClientRect(m_WindowHandle, &client);
        OnResize(client.right, client.bottom, client.right, client.bottom);
        m_IsOpen     = true;
        m_IsPrepared = isPrepared;
        DragAcceptFiles(m_WindowHandle, TRUE);
        ShowWindow(m_WindowHandle, SW_SHOWDEFAULT);
        UpdateWindow(m_WindowHandle);
    }

    void Window::Destroy() noexcept {
        Clear();
    }

    void Window::PollEvents() noexcept {
        MSG message{};
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }

    void Window::WaitEvents() noexcept {
        MsgWaitForMultipleObjectsEx(0, nullptr, INFINITE, QS_ALLINPUT, MWMO_INPUTAVAILABLE);
        PollEvents();
    }

    void Window::SetTitle(const char* title) noexcept {
        const auto wide{ Wide(title) };
        SetWindowTextW(GetHandle(), wide.c_str());
    }

    void Window::Restore() noexcept {
        if (m_IsMinimized) {
            ShowWindow(GetHandle(), SW_RESTORE);
        }
    }

    void Window::SetCursor(Cursor cursor) noexcept {
        const LPCWSTR names[]{ IDC_ARROW, IDC_IBEAM, IDC_SIZEALL, IDC_SIZENS, IDC_SIZEWE, IDC_SIZENESW, IDC_SIZENWSE, IDC_HAND, IDC_NO };
        m_Cursor = cursor == Cursor::eHidden ? nullptr : LoadCursorW(nullptr, names[static_cast<std::size_t>(cursor)]);
        ::SetCursor(m_Cursor);
    }

    std::string Window::GetClipboardText() {
        if (!OpenClipboard(GetHandle())) {
            return {};
        }
        std::string value;
        if (HANDLE memory{ GetClipboardData(CF_UNICODETEXT) }) {
            if (auto* text{ static_cast<const wchar_t*>(GlobalLock(memory)) }) {
                value = Utf8(text);
                GlobalUnlock(memory);
            }
        }
        CloseClipboard();
        return value;
    }

    void Window::SetClipboardText(const char* text) {
        const auto value{ Wide(text) };
        const std::size_t bytes{ (value.size() + 1) * sizeof(wchar_t) };
        HGLOBAL memory{ GlobalAlloc(GMEM_MOVEABLE, bytes) };
        if (!memory) {
            return;
        }
        void* destination{ GlobalLock(memory) };
        if (!destination) {
            GlobalFree(memory);
            return;
        }
        std::memcpy(destination, value.c_str(), bytes);
        GlobalUnlock(memory);
        if (OpenClipboard(GetHandle())) {
            EmptyClipboard();
            if (SetClipboardData(CF_UNICODETEXT, memory)) {
                memory = nullptr;
            }
            CloseClipboard();
        }
        if (memory) {
            GlobalFree(memory);
        }
    }

    LRESULT WINAPI Window::WindowProcedure(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) noexcept {
        auto* window{ reinterpret_cast<Window*>(GetWindowLongPtrW(handle, GWLP_USERDATA)) };
        if (message == WM_NCCREATE) {
            window = static_cast<Window*>(reinterpret_cast<CREATESTRUCTW*>(lParam)->lpCreateParams);
            SetWindowLongPtrW(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
            window->m_WindowHandle = handle;
        }
        if (!window) {
            return DefWindowProcW(handle, message, wParam, lParam);
        }
        switch (message) {
        case WM_CLOSE:
            EventBus().publish<WindowEvent>(WindowEvent::Type::eCloseRequested);
            return 0;
        case WM_QUERYENDSESSION:
            EventBus().publish<WindowEvent>(WindowEvent::Type::eCloseRequested);
            return FALSE;
        case WM_SIZE:
            window->OnMinimized(wParam == SIZE_MINIMIZED);
            if (wParam != SIZE_MINIMIZED) {
                RECT client{};
                GetClientRect(handle, &client);
                window->OnResize(client.right, client.bottom, client.right, client.bottom);
            }
            return 0;
        case WM_DPICHANGED:
            {
                const auto* bounds{ reinterpret_cast<const RECT*>(lParam) };
                SetWindowPos(handle, nullptr, bounds->left, bounds->top, bounds->right - bounds->left, bounds->bottom - bounds->top, SWP_NOACTIVATE | SWP_NOZORDER);
                return 0;
            }
        case WM_SETFOCUS:
            window->OnFocus(true);
            return 0;
        case WM_KILLFOCUS:
            window->m_HighSurrogate = 0;
            window->m_Buttons       = 0;
            if (GetCapture() == handle) {
                ReleaseCapture();
            }
            window->OnFocus(false);
            return 0;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
            {
                const bool down{ message == WM_KEYDOWN || message == WM_SYSKEYDOWN };
                const bool extended{ (lParam & (1L << 24)) != 0 };
                Keycode key{ wParam };
                if (key == VK_SHIFT) {
                    key = MapVirtualKeyW(static_cast<UINT>((lParam >> 16) & 0xFF), MAPVK_VSC_TO_VK_EX);
                } else if (key == VK_CONTROL) {
                    key = extended ? VK_RCONTROL : VK_LCONTROL;
                } else if (key == VK_MENU) {
                    key = extended ? VK_RMENU : VK_LMENU;
                } else if (key == VK_RETURN && extended) {
                    key = ADH_NUMPAD_ENTER;
                }
                EventBus().publish<KeyboardEvent>(down ? ((lParam & (1L << 30)) ? KeyboardEvent::Type::eKeyRepeat : KeyboardEvent::Type::eKeyDown) : KeyboardEvent::Type::eKeyUp, key);
                if (message == WM_SYSKEYDOWN && wParam == VK_F4) {
                    return DefWindowProcW(handle, message, wParam, lParam);
                }
                return 0;
            }
        case WM_CHAR:
            {
                const auto value{ static_cast<std::uint32_t>(wParam) };
                if (value >= 0xD800 && value <= 0xDBFF) {
                    window->m_HighSurrogate = value;
                    return 0;
                }
                if (value >= 0xDC00 && value <= 0xDFFF && window->m_HighSurrogate) {
                    PublishCharacter(0x10000 + ((window->m_HighSurrogate - 0xD800) << 10) + value - 0xDC00);
                } else {
                    PublishCharacter(value);
                }
                window->m_HighSurrogate = 0;
                return 0;
            }
        case WM_MOUSEMOVE:
            if (!window->m_IsMouseTracked) {
                TRACKMOUSEEVENT tracking{ sizeof(TRACKMOUSEEVENT), TME_LEAVE, handle, 0 };
                window->m_IsMouseTracked = TrackMouseEvent(&tracking) != FALSE;
            }
            EventBus().publish<MouseMoveEvent>(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        case WM_MOUSELEAVE:
            window->m_IsMouseTracked = false;
            if (!window->m_Buttons) {
                EventBus().publish<MouseMoveEvent>(std::numeric_limits<std::int16_t>::min(), std::numeric_limits<std::int16_t>::min());
            }
            return 0;
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
            {
                using Index = MouseButtonEvent::Index;
                using Type  = MouseButtonEvent::Type;
                const bool down{ message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN || message == WM_MBUTTONDOWN };
                const unsigned button{ message == WM_LBUTTONDOWN || message == WM_LBUTTONUP ? 0u : (message == WM_RBUTTONDOWN || message == WM_RBUTTONUP ? 1u : 2u) };
                constexpr Index indices[]{ Index::eLeftButton, Index::eRightButton, Index::eMiddleButton };
                constexpr Type downs[]{ Type::eLeftButtonDown, Type::eRightButtonDown, Type::eMiddleButtonDown };
                constexpr Type ups[]{ Type::eLeftButtonUp, Type::eRightButtonUp, Type::eMiddleButtonUp };
                if (down) {
                    window->m_Buttons |= 1u << button;
                    SetCapture(handle);
                } else {
                    window->m_Buttons &= ~(1u << button);
                    if (!window->m_Buttons && GetCapture() == handle) {
                        ReleaseCapture();
                    }
                }
                EventBus().publish<MouseButtonEvent>(indices[button], down ? downs[button] : ups[button], GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
                return 0;
            }
        case WM_CAPTURECHANGED:
            if (window->m_Buttons) {
                POINT point{};
                GetCursorPos(&point);
                ScreenToClient(handle, &point);
                constexpr MouseButtonEvent::Type ups[]{ MouseButtonEvent::Type::eLeftButtonUp, MouseButtonEvent::Type::eRightButtonUp, MouseButtonEvent::Type::eMiddleButtonUp };
                for (unsigned button{}; button < 3; ++button) {
                    if (window->m_Buttons & (1u << button)) {
                        EventBus().publish<MouseButtonEvent>(static_cast<MouseButtonEvent::Index>(button), ups[button], point.x, point.y);
                    }
                }
                window->m_Buttons = 0;
            }
            return 0;
        case WM_MOUSEWHEEL:
            {
                POINT point{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                ScreenToClient(handle, &point);
                EventBus().publish<MouseWheelEvent>(static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA, point.x, point.y);
                return 0;
            }
        case WM_SETCURSOR:
            if (LOWORD(lParam) == HTCLIENT) {
                ::SetCursor(window->m_Cursor);
                return TRUE;
            }
            break;
        case WM_DROPFILES:
            {
                const HDROP drop{ reinterpret_cast<HDROP>(wParam) };
                const UINT count{ DragQueryFileW(drop, 0xFFFFFFFF, nullptr, 0) };
                for (UINT i{}; i < count; ++i) {
                    const UINT length{ DragQueryFileW(drop, i, nullptr, 0) };
                    std::wstring path(length + 1, L'\0');
                    DragQueryFileW(drop, i, path.data(), length + 1);
                    const auto utf8{ Utf8(path.c_str()) };
                    EventBus().publish<WindowDropEvent>(WindowEvent::Type::eDrop, utf8.c_str());
                }
                DragFinish(drop);
                return 0;
            }
        case WM_DESTROY:
            window->SetOpen(false);
            return 0;
        case WM_NCDESTROY:
            SetWindowLongPtrW(handle, GWLP_USERDATA, 0);
            window->m_WindowHandle = nullptr;
            break;
        }
        return DefWindowProcW(handle, message, wParam, lParam);
    }

    void Window::Clear() noexcept {
        if (m_WindowHandle) {
            DestroyWindow(m_WindowHandle);
            m_WindowHandle = nullptr;
        }
    }
} // namespace adh
