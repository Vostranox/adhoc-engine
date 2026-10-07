#pragma once

#if defined(ADH_WINDOWS)
#    include <Windows.h>
#elif defined(ADH_LINUX)
#    include <X11/Xlib.h>
#    include <xcb/xcb.h>
#endif

#include <array>
#include <bitset>
#include <cstdint>
#include <string>

namespace adh {
    class Window {
      public:
        enum class Cursor {
            eArrow,
            eTextInput,
            eResizeAll,
            eResizeNS,
            eResizeEW,
            eResizeNESW,
            eResizeNWSE,
            eHand,
            eNotAllowed,
            eHidden
        };

      public:
        Window() noexcept = default;

        Window(const Window&) = delete;

        Window& operator=(const Window&) = delete;

        Window(Window&&) = delete;

        Window& operator=(Window&&) = delete;

        ~Window();

        void Create(const char* name, std::int32_t width, std::int32_t height, bool isPrepared, bool setFullscreen);

        void Destroy() noexcept;

        void PollEvents() noexcept;

        void WaitEvents() noexcept;

        bool IsOpen() const noexcept;

        bool IsMinimized() const noexcept;

        bool IsInFocus() const noexcept;

        void SetOpen(bool isOpen) noexcept;

        std::int32_t GetWindowWidth() const noexcept;

        std::int32_t GetWindowHeight() const noexcept;

        std::int32_t GetLogicalWidth() const noexcept;

        std::int32_t GetLogicalHeight() const noexcept;

        void SetTitle(const char* title) noexcept;

        void Restore() noexcept;

        std::string GetClipboardText();

        void SetClipboardText(const char* text);

        void SetCursor(Cursor cursor) noexcept;

        void OnResize(std::int32_t logicalWidth, std::int32_t logicalHeight, std::int32_t pixelWidth, std::int32_t pixelHeight);

        void OnFocus(bool focused);

        void OnMinimized(bool minimized);

      private:
        void Clear() noexcept;

      private:
        bool m_IsOpen{};
        bool m_IsMinimized{};
        bool m_IsInFocus{};
        bool m_IsPrepared{};
        std::int32_t m_Width{};
        std::int32_t m_Height{};
        std::int32_t m_LogicalWidth{};
        std::int32_t m_LogicalHeight{};

#if defined(ADH_WINDOWS)
      public:
        HWND GetHandle() const noexcept;

        static HINSTANCE GetInstance() noexcept;

      public:
        static LRESULT WINAPI WindowProcedure(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) noexcept;

      private:
        HWND m_WindowHandle{};
        HCURSOR m_Cursor{};
        std::uint32_t m_HighSurrogate{};
        std::uint32_t m_Buttons{};
        bool m_IsMouseTracked{};

#elif defined(ADH_APPLE)
      public:
        void* GetHandle() const noexcept;

      private:
        struct Native;
        Native* m_Native{};

#elif defined(ADH_LINUX)
      public:
        xcb_connection_t* GetConnection() const noexcept;
        xcb_window_t GetWindow() const noexcept;

      private:
        void HandleEvent(XEvent& event);

        std::string ReadProperty(Atom property);

      private:
        Display* m_Display{};
        xcb_connection_t* m_Connection{};
        ::Window m_Window{};
        XIM m_InputMethod{};
        XIC m_InputContext{};
        std::bitset<256> m_KeysDown;
        Atom m_Protocols{};
        Atom m_DeleteWindow{};
        Atom m_Utf8{};
        Atom m_Clipboard{};
        Atom m_Targets{};
        Atom m_ClipboardProperty{};
        std::string m_ClipboardText;
        std::string m_ReceivedClipboard;
        bool m_IsClipboardWaiting{};
        std::array<::Cursor, 10> m_Cursors{};
        std::uint32_t m_MouseButtons{};
#endif
    };
} // namespace adh
