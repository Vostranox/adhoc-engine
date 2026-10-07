#include "Window.hpp"

#include <Event/Event.hpp>

namespace adh {
    bool Window::IsOpen() const noexcept {
        return m_IsOpen;
    }

    bool Window::IsMinimized() const noexcept {
        return m_IsMinimized;
    }

    bool Window::IsInFocus() const noexcept {
        return m_IsInFocus;
    }

    void Window::SetOpen(bool isOpen) noexcept {
        m_IsOpen = isOpen;
    }

    std::int32_t Window::GetWindowWidth() const noexcept {
        return m_Width;
    }

    std::int32_t Window::GetWindowHeight() const noexcept {
        return m_Height;
    }

    std::int32_t Window::GetLogicalWidth() const noexcept {
        return m_LogicalWidth;
    }

    std::int32_t Window::GetLogicalHeight() const noexcept {
        return m_LogicalHeight;
    }

    void Window::OnResize(std::int32_t logicalWidth, std::int32_t logicalHeight, std::int32_t pixelWidth, std::int32_t pixelHeight) {
        if (logicalWidth <= 0 || logicalHeight <= 0 || pixelWidth <= 0 || pixelHeight <= 0) {
            return;
        }
        const bool changed{ pixelWidth != m_Width || pixelHeight != m_Height };
        m_LogicalWidth  = logicalWidth;
        m_LogicalHeight = logicalHeight;
        m_Width         = pixelWidth;
        m_Height        = pixelHeight;
        if (changed && m_IsPrepared) {
            EventBus().publish<WindowEvent>(WindowEvent::Type::eResized);
        }
    }

    void Window::OnFocus(bool focused) {
        if (focused != m_IsInFocus) {
            m_IsInFocus = focused;
            EventBus().publish<WindowEvent>(focused ? WindowEvent::Type::eFocus : WindowEvent::Type::eKillfocus);
        }
    }

    void Window::OnMinimized(bool minimized) {
        m_IsMinimized = minimized;
    }
} // namespace adh
