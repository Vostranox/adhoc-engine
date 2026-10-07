#pragma once
#include <Utility.hpp>
#include <Window.hpp>

#include <vulkan/vulkan.h>

namespace adh {
    namespace vk {
        class Surface {
          public:
            Surface() noexcept;

            Surface(VkInstance instance, const Window& window) ADH_NOEXCEPT;

            Surface(const Surface&) = delete;

            Surface& operator=(const Surface&) = delete;

            Surface(Surface&& rhs) noexcept;

            Surface& operator=(Surface&& rhs) noexcept;

            ~Surface();

            void Create(VkInstance instance, const Window& window) ADH_NOEXCEPT;

            void Destroy() noexcept;

            operator VkSurfaceKHR() noexcept;

            operator VkSurfaceKHR() const noexcept;

          private:
            void MoveConstruct(Surface&& rhs) noexcept;

            void Clear() noexcept;

          private:
            VkSurfaceKHR m_Surface;
            VkInstance m_Instance;
        };
    } // namespace vk
} // namespace adh
