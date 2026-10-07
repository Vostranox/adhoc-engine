#include "Surface.hpp"
#include "Initializers.hpp"

#include <Std/Utility.hpp>
#include <Utility.hpp>

namespace adh {
    namespace vk {
        Surface::Surface() noexcept : m_Surface{ VK_NULL_HANDLE },
                                      m_Instance{ VK_NULL_HANDLE } {
        }

        Surface::Surface(VkInstance instance, const Window& window) ADH_NOEXCEPT {
            Create(instance, window);
        }

        Surface::Surface(Surface&& rhs) noexcept {
            MoveConstruct(Move(rhs));
        }

        Surface& Surface::operator=(Surface&& rhs) noexcept {
            Clear();
            MoveConstruct(Move(rhs));

            return *this;
        }

        Surface::~Surface() {
            Clear();
        }

        void Surface::Create(VkInstance instance, const Window& window) ADH_NOEXCEPT {
            m_Instance = instance;
#if defined(ADH_WINDOWS)
            auto info{ initializers::Win32SurfaceCreateInfo(window.GetHandle(), window.GetInstance()) };
            ADH_THROW(vkCreateWin32SurfaceKHR(instance, &info, nullptr, &m_Surface) == VK_SUCCESS,
                      "Failed to create window surface!");
#elif defined(ADH_APPLE)
            auto info{ initializers::MacOSSurfaceCreateInfo(window.GetHandle()) };
            auto create{ reinterpret_cast<PFN_vkCreateMacOSSurfaceMVK>(vkGetInstanceProcAddr(instance, "vkCreateMacOSSurfaceMVK")) };
            ADH_THROW(create && create(instance, &info, nullptr, &m_Surface) == VK_SUCCESS,
                      "Failed to create window surface!");
#elif defined(ADH_LINUX)
            auto info{ initializers::XcbSurfaceCreateInfo(window.GetConnection(), window.GetWindow()) };
            ADH_THROW(vkCreateXcbSurfaceKHR(instance, &info, nullptr, &m_Surface) == VK_SUCCESS,
                      "Failed to create window surface!");
#endif
        }

        void Surface::Destroy() noexcept {
            Clear();
        }

        Surface::operator VkSurfaceKHR() noexcept {
            return m_Surface;
        }

        Surface::operator VkSurfaceKHR() const noexcept {
            return m_Surface;
        }

        void Surface::MoveConstruct(Surface&& rhs) noexcept {
            m_Surface     = rhs.m_Surface;
            rhs.m_Surface = VK_NULL_HANDLE;
        }

        void Surface::Clear() noexcept {
            if (m_Surface != VK_NULL_HANDLE) {
                vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
                m_Surface = VK_NULL_HANDLE;
            }
        }
    } // namespace vk
} // namespace adh
