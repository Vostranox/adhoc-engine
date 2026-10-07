#include <Window.hpp>

#include <Event/Event.hpp>
#include <Input/Keycodes.hpp>

#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <X11/Xlib-xcb.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/joystick.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <array>
#include <bitset>
#include <cerrno>
#include <chrono>
#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>

namespace adh {
    static void PublishText(const char* text, int length) {
        for (int i{}; i < length;) {
            std::uint32_t value{ static_cast<unsigned char>(text[i++]) };
            int remaining{};
            if (value >= 0xF0) {
                value &= 0x07u;
                remaining = 3;
            } else if (value >= 0xE0) {
                value &= 0x0Fu;
                remaining = 2;
            } else if (value >= 0xC0) {
                value &= 0x1Fu;
                remaining = 1;
            }
            for (; remaining > 0 && i < length; --remaining) {
                value = (value << 6u) | (static_cast<unsigned char>(text[i++]) & 0x3Fu);
            }
            if (value >= 32 && value != 127) {
                EventBus().publish<CharEvent>(value);
            }
        }
    }

    static constexpr std::uint32_t maxControllers{ 4u };

    struct Joystick {
        int fd{ -1 };
        std::array<unsigned char, ABS_CNT> axes{};
        std::array<unsigned short, KEY_MAX - BTN_MISC + 1> buttons{};
        std::array<bool, ADH_BUTTON_COUNT> pressed{};
        std::array<bool, ADH_BUTTON_COUNT> reported{};
    };

    static std::array<Joystick, maxControllers> joysticks;

    static void PollControllers(bool focused) {
        for (std::uint32_t id{}; id < joysticks.size(); ++id) {
            auto& joystick{ joysticks[id] };
            if (joystick.fd < 0) {
                const std::string path{ "/dev/input/js" + std::to_string(id) };
                joystick.fd = open(path.c_str(), O_RDONLY | O_NONBLOCK | O_CLOEXEC);
                if (joystick.fd >= 0) {
                    if (ioctl(joystick.fd, JSIOCGAXMAP, joystick.axes.data()) < 0 || ioctl(joystick.fd, JSIOCGBTNMAP, joystick.buttons.data()) < 0) {
                        close(joystick.fd);
                        joystick.fd = -1;
                    }
                }
            }
            if (joystick.fd >= 0) {
                js_event event{};
                ssize_t count{};
                while ((count = read(joystick.fd, &event, sizeof(event))) == sizeof(event)) {
                    const unsigned kind{ static_cast<unsigned>(event.type & ~JS_EVENT_INIT) };
                    if (kind == JS_EVENT_BUTTON) {
                        constexpr unsigned short codes[]{ BTN_SOUTH, BTN_EAST, BTN_WEST, BTN_NORTH, BTN_SELECT, BTN_START, BTN_THUMBL, BTN_THUMBR, BTN_TL, BTN_TR, BTN_DPAD_UP, BTN_DPAD_DOWN, BTN_DPAD_LEFT, BTN_DPAD_RIGHT, BTN_TL2, BTN_TR2 };
                        for (std::size_t button{}; button < std::size(codes); ++button) {
                            if (codes[button] == joystick.buttons[event.number]) {
                                joystick.pressed[button] = event.value != 0;
                            }
                        }
                    } else if (kind == JS_EVENT_AXIS && event.number < joystick.axes.size()) {
                        switch (joystick.axes[event.number]) {
                        case ABS_HAT0X:
                            joystick.pressed[ADH_BUTTON_DPAD_LEFT]  = event.value < 0;
                            joystick.pressed[ADH_BUTTON_DPAD_RIGHT] = event.value > 0;
                            break;
                        case ABS_HAT0Y:
                            joystick.pressed[ADH_BUTTON_DPAD_UP]   = event.value < 0;
                            joystick.pressed[ADH_BUTTON_DPAD_DOWN] = event.value > 0;
                            break;
                        case ABS_Z:
                            joystick.pressed[ADH_BUTTON_LTRIGGER] = event.value > -25000;
                            break;
                        case ABS_RZ:
                            joystick.pressed[ADH_BUTTON_RTRIGGER] = event.value > -25000;
                            break;
                        }
                    }
                }
                if (count == 0 || (count < 0 && errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)) {
                    close(joystick.fd);
                    joystick.fd = -1;
                    joystick.pressed.fill(false);
                }
            }
            for (std::size_t button{}; button < joystick.pressed.size(); ++button) {
                const bool pressed{ focused && joystick.pressed[button] };
                if (pressed) {
                    EventBus().publish<ControllerEvent>(joystick.reported[button] ? ControllerEvent::Type::eButtonRepeat : ControllerEvent::Type::eButtonDown, button, id);
                } else if (joystick.reported[button]) {
                    EventBus().publish<ControllerEvent>(ControllerEvent::Type::eButtonUp, button, id);
                }
                joystick.reported[button] = pressed;
            }
        }
    }

    std::string Window::ReadProperty(Atom property) {
        Atom type{};
        int format{};
        unsigned long count{};
        unsigned long after{};
        unsigned char* bytes{};
        const int result{ XGetWindowProperty(m_Display, m_Window, property, 0, 4 * 1024 * 1024, True, AnyPropertyType, &type, &format, &count, &after, &bytes) };
        std::string text;
        if (result == Success && format == 8 && bytes && after == 0) {
            text.assign(reinterpret_cast<char*>(bytes), count);
        }
        if (bytes) {
            XFree(bytes);
        }
        return text;
    }

    void Window::HandleEvent(XEvent& event) {
        if (XFilterEvent(&event, None)) {
            return;
        }
        if (event.type == SelectionRequest) {
            const auto& request{ event.xselectionrequest };
            XEvent answer{};
            answer.xselection = { SelectionNotify, 0, True, m_Display, request.requestor, request.selection, request.target, None, request.time };
            const Atom property{ request.property == None ? request.target : request.property };
            if (request.selection == m_Clipboard && XGetSelectionOwner(m_Display, m_Clipboard) == m_Window) {
                if (request.target == m_Targets) {
                    const Atom supported[]{ m_Targets, m_Utf8, XA_STRING };
                    XChangeProperty(m_Display, request.requestor, property, XA_ATOM, 32, PropModeReplace, reinterpret_cast<const unsigned char*>(supported), 3);
                    answer.xselection.property = property;
                } else if ((request.target == m_Utf8 || request.target == XA_STRING) && m_ClipboardText.size() < static_cast<std::size_t>(XMaxRequestSize(m_Display) - 64) * 4) {
                    XChangeProperty(m_Display, request.requestor, property, request.target, 8, PropModeReplace, reinterpret_cast<const unsigned char*>(m_ClipboardText.data()), static_cast<int>(m_ClipboardText.size()));
                    answer.xselection.property = property;
                }
            }
            XSendEvent(m_Display, request.requestor, False, NoEventMask, &answer);
            XFlush(m_Display);
            return;
        }
        if (event.type == SelectionNotify) {
            if (event.xselection.selection == m_Clipboard && m_IsClipboardWaiting) {
                if (event.xselection.property != None) {
                    m_ReceivedClipboard = ReadProperty(event.xselection.property);
                }
                m_IsClipboardWaiting = false;
            }
            return;
        }
        if (event.type == ClientMessage) {
            auto& message{ event.xclient };
            if (message.message_type == m_Protocols && static_cast<Atom>(message.data.l[0]) == m_DeleteWindow) {
                EventBus().publish<WindowEvent>(WindowEvent::Type::eCloseRequested);
            }
            return;
        }
        if (event.type == KeyPress || event.type == KeyRelease) {
            const auto code{ event.xkey.keycode };
            if (code < m_KeysDown.size()) {
                const auto kind{ event.type == KeyRelease ? KeyboardEvent::Type::eKeyUp : (m_KeysDown[code] ? KeyboardEvent::Type::eKeyRepeat : KeyboardEvent::Type::eKeyDown) };
                m_KeysDown[code] = event.type == KeyPress;
                EventBus().publish<KeyboardEvent>(kind, code);
            }
        }
        switch (event.type) {
        case ConfigureNotify:
            OnResize(event.xconfigure.width, event.xconfigure.height, event.xconfigure.width, event.xconfigure.height);
            break;
        case MapNotify:
            OnMinimized(false);
            break;
        case UnmapNotify:
            OnMinimized(true);
            break;
        case FocusIn:
            if (event.xfocus.detail != NotifyInferior && event.xfocus.mode != NotifyGrab && event.xfocus.mode != NotifyUngrab) {
                if (m_InputContext) {
                    XSetICFocus(m_InputContext);
                }
                OnFocus(true);
            }
            break;
        case FocusOut:
            if (event.xfocus.detail != NotifyInferior && event.xfocus.mode != NotifyGrab && event.xfocus.mode != NotifyUngrab) {
                m_KeysDown.reset();
                m_MouseButtons = 0;
                if (m_InputContext) {
                    XUnsetICFocus(m_InputContext);
                }
                OnFocus(false);
            }
            break;
        case KeyPress:
            {
                std::array<char, 256> buffer{};
                KeySym symbol{};
                Status status{};
                if (m_InputContext) {
                    const int length{ Xutf8LookupString(m_InputContext, &event.xkey, buffer.data(), static_cast<int>(buffer.size()), &symbol, &status) };
                    if (status == XLookupChars || status == XLookupBoth) {
                        PublishText(buffer.data(), length);
                    }
                } else {
                    const int count{ XLookupString(&event.xkey, buffer.data(), static_cast<int>(buffer.size()), &symbol, nullptr) };
                    for (int i{}; i < count; ++i) {
                        const auto value{ static_cast<unsigned char>(buffer[i]) };
                        if (value >= 32 && value != 127) {
                            EventBus().publish<CharEvent>(value);
                        }
                    }
                }
                break;
            }
        case MotionNotify:
            EventBus().publish<MouseMoveEvent>(event.xmotion.x, event.xmotion.y);
            break;
        case EnterNotify:
            if (event.xcrossing.mode == NotifyNormal) {
                EventBus().publish<MouseMoveEvent>(event.xcrossing.x, event.xcrossing.y);
            }
            break;
        case LeaveNotify:
            if (event.xcrossing.mode == NotifyNormal && !m_MouseButtons) {
                EventBus().publish<MouseMoveEvent>(std::numeric_limits<std::int16_t>::min(), std::numeric_limits<std::int16_t>::min());
            }
            break;
        case ButtonPress:
        case ButtonRelease:
            {
                const bool down{ event.type == ButtonPress };
                if (event.xbutton.button <= Button3) {
                    if (down) {
                        m_MouseButtons |= 1u << event.xbutton.button;
                    } else {
                        m_MouseButtons &= ~(1u << event.xbutton.button);
                    }
                }
                const auto x{ static_cast<std::int16_t>(event.xbutton.x) };
                const auto y{ static_cast<std::int16_t>(event.xbutton.y) };
                using Index = MouseButtonEvent::Index;
                using Type  = MouseButtonEvent::Type;
                switch (event.xbutton.button) {
                case Button1:
                    EventBus().publish<MouseButtonEvent>(Index::eLeftButton, down ? Type::eLeftButtonDown : Type::eLeftButtonUp, x, y);
                    break;
                case Button2:
                    EventBus().publish<MouseButtonEvent>(Index::eMiddleButton, down ? Type::eMiddleButtonDown : Type::eMiddleButtonUp, x, y);
                    break;
                case Button3:
                    EventBus().publish<MouseButtonEvent>(Index::eRightButton, down ? Type::eRightButtonDown : Type::eRightButtonUp, x, y);
                    break;
                case Button4:
                    if (down) {
                        EventBus().publish<MouseWheelEvent>(1.0f, x, y);
                    }
                    break;
                case Button5:
                    if (down) {
                        EventBus().publish<MouseWheelEvent>(-1.0f, x, y);
                    }
                    break;
                }
                break;
            }
        }
    }

    Window::~Window() {
        Clear();
    }

    void Window::Create(const char* name, std::int32_t width, std::int32_t height, bool isPrepared, bool) {
        std::setlocale(LC_CTYPE, "");
        XSetLocaleModifiers("");
        m_Display = XOpenDisplay(nullptr);
        if (!m_Display) {
            std::fputs("AdHoc: cannot open the X11 display\n", stderr);
            std::exit(EXIT_FAILURE);
        }
        m_Connection = XGetXCBConnection(m_Display);
        XkbSetDetectableAutoRepeat(m_Display, True, nullptr);
        const int screen{ DefaultScreen(m_Display) };
        m_Window = XCreateSimpleWindow(m_Display, RootWindow(m_Display, screen), 0, 0, width, height, 0, 0, BlackPixel(m_Display, screen));
        XSelectInput(m_Display, m_Window, StructureNotifyMask | KeyPressMask | KeyReleaseMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask | FocusChangeMask | EnterWindowMask | LeaveWindowMask);
        m_Protocols         = XInternAtom(m_Display, "WM_PROTOCOLS", False);
        m_DeleteWindow      = XInternAtom(m_Display, "WM_DELETE_WINDOW", False);
        m_Utf8              = XInternAtom(m_Display, "UTF8_STRING", False);
        m_Clipboard         = XInternAtom(m_Display, "CLIPBOARD", False);
        m_Targets           = XInternAtom(m_Display, "TARGETS", False);
        m_ClipboardProperty = XInternAtom(m_Display, "ADHOC_CLIPBOARD", False);
        XSetWMProtocols(m_Display, m_Window, &m_DeleteWindow, 1);
        m_InputMethod = XOpenIM(m_Display, nullptr, nullptr, nullptr);
        if (!m_InputMethod) {
            XSetLocaleModifiers("@im=none");
            m_InputMethod = XOpenIM(m_Display, nullptr, nullptr, nullptr);
        }
        if (m_InputMethod) {
            m_InputContext = XCreateIC(m_InputMethod, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, m_Window, XNFocusWindow, m_Window, nullptr);
        }
        SetTitle(name);
        OnResize(width, height, width, height);
        m_IsOpen = true;
        XMapWindow(m_Display, m_Window);
        XFlush(m_Display);
        m_IsPrepared = isPrepared;
        SetCursor(Cursor::eArrow);
    }

    void Window::Destroy() noexcept {
        Clear();
    }

    void Window::PollEvents() noexcept {
        while (XPending(m_Display)) {
            XEvent event{};
            XNextEvent(m_Display, &event);
            HandleEvent(event);
        }
        PollControllers(m_IsInFocus);
    }

    void Window::WaitEvents() noexcept {
        XEvent event{};
        XNextEvent(m_Display, &event);
        HandleEvent(event);
        PollEvents();
    }

    xcb_connection_t* Window::GetConnection() const noexcept {
        return m_Connection;
    }

    xcb_window_t Window::GetWindow() const noexcept {
        return static_cast<xcb_window_t>(m_Window);
    }

    void Window::SetTitle(const char* title) noexcept {
        XStoreName(m_Display, m_Window, title);
        XChangeProperty(m_Display, m_Window, XInternAtom(m_Display, "_NET_WM_NAME", False), m_Utf8, 8, PropModeReplace, reinterpret_cast<const unsigned char*>(title), static_cast<int>(std::strlen(title)));
        XFlush(m_Display);
    }

    void Window::Restore() noexcept {
        if (m_IsMinimized) {
            XMapRaised(m_Display, m_Window);
            XFlush(m_Display);
        }
    }

    void Window::SetClipboardText(const char* text) {
        m_ClipboardText = text;
        XSetSelectionOwner(m_Display, m_Clipboard, m_Window, CurrentTime);
        XFlush(m_Display);
    }

    std::string Window::GetClipboardText() {
        const auto owner{ XGetSelectionOwner(m_Display, m_Clipboard) };
        if (owner == m_Window) {
            return m_ClipboardText;
        }
        if (owner == None) {
            return {};
        }
        m_ReceivedClipboard.clear();
        m_IsClipboardWaiting = true;
        XConvertSelection(m_Display, m_Clipboard, m_Utf8, m_ClipboardProperty, m_Window, CurrentTime);
        XFlush(m_Display);
        const auto deadline{ std::chrono::steady_clock::now() + std::chrono::seconds{ 1 } };
        while (m_IsClipboardWaiting && std::chrono::steady_clock::now() < deadline) {
            PollEvents();
            if (m_IsClipboardWaiting) {
                pollfd connection{ ConnectionNumber(m_Display), POLLIN, 0 };
                poll(&connection, 1, 10);
            }
        }
        m_IsClipboardWaiting = false;
        return m_ReceivedClipboard;
    }

    void Window::SetCursor(Cursor cursor) noexcept {
        const auto index{ static_cast<std::size_t>(cursor) };
        if (!m_Cursors[index]) {
            constexpr unsigned shapes[]{ XC_left_ptr, XC_xterm, XC_fleur, XC_sb_v_double_arrow, XC_sb_h_double_arrow, XC_bottom_left_corner, XC_bottom_right_corner, XC_hand2, XC_X_cursor };
            if (cursor == Cursor::eHidden) {
                const char pixel{};
                const Pixmap bitmap{ XCreateBitmapFromData(m_Display, m_Window, &pixel, 1, 1) };
                XColor color{};
                m_Cursors[index] = XCreatePixmapCursor(m_Display, bitmap, bitmap, &color, &color, 0, 0);
                XFreePixmap(m_Display, bitmap);
            } else {
                m_Cursors[index] = XCreateFontCursor(m_Display, shapes[index]);
            }
        }
        XDefineCursor(m_Display, m_Window, m_Cursors[index]);
    }

    void Window::Clear() noexcept {
        for (auto& joystick : joysticks) {
            if (joystick.fd >= 0) {
                close(joystick.fd);
                joystick.fd = -1;
            }
        }
        if (!m_Display) {
            return;
        }
        if (m_InputContext) {
            XDestroyIC(m_InputContext);
            m_InputContext = nullptr;
        }
        if (m_InputMethod) {
            XCloseIM(m_InputMethod);
            m_InputMethod = nullptr;
        }
        for (auto& cursor : m_Cursors) {
            if (cursor) {
                XFreeCursor(m_Display, cursor);
                cursor = 0;
            }
        }
        if (m_Window) {
            XDestroyWindow(m_Display, m_Window);
            m_Window = 0;
        }
        XCloseDisplay(m_Display);
        m_Display    = nullptr;
        m_Connection = nullptr;
    }
} // namespace adh
