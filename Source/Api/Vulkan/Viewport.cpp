#include "Viewport.hpp"
#include "Initializers.hpp"

namespace adh {
    namespace vk {
        Viewport::Viewport(float width, float height, bool flipYAxis) noexcept {
            Create(width, height, flipYAxis);
        }

        Viewport::Viewport(VkExtent2D extent, bool flipYAxis) noexcept {
            Create(extent, flipYAxis);
        }

        void Viewport::Create(float width, float height, bool flipYAxis) noexcept {
            if (flipYAxis) {
                m_Viewport = initializers::Viewport(0.0f, height, width, -height);
            } else {
                m_Viewport = initializers::Viewport(0.0f, 0.0f, width, height);
            }
        }

        void Viewport::Create(VkExtent2D extent, bool flipYAxis) noexcept {
            float width{ static_cast<float>(extent.width) };
            float height{ static_cast<float>(extent.height) };
            if (flipYAxis) {
                m_Viewport = initializers::Viewport(0.0f, height, width, -height);
            } else {
                m_Viewport = initializers::Viewport(0.0f, 0.0f, width, height);
            }
        }

        Viewport::Viewport(const VkViewport& viewport) noexcept {
            m_Viewport = viewport;
        }

        void Viewport::Create(const VkViewport& viewport) noexcept {
            m_Viewport = viewport;
        }

        void Viewport::Update(float width, float height, bool flipYAxis) noexcept {
            Create(width, height, flipYAxis);
        }

        void Viewport::Update(VkExtent2D extent, bool flipYAxis) noexcept {
            Create(extent, flipYAxis);
        }

        void Viewport::Update(const VkViewport& viewport) noexcept {
            Create(viewport);
        }

        void Viewport::Set(VkCommandBuffer commandBuffer) noexcept {
            vkCmdSetViewport(commandBuffer, 0u, 1u, &m_Viewport);
        }
    } // namespace vk
} // namespace adh
