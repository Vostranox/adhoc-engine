#pragma once
#include <vulkan/vulkan.h>

namespace adh {
    namespace vk {
        class Viewport {
          public:
            Viewport() noexcept = default;

            Viewport(float width, float height, bool flipYAxis) noexcept;

            Viewport(VkExtent2D extent, bool flipYAxis) noexcept;

            Viewport(const VkViewport& viewport) noexcept;

            void Create(float width, float height, bool flipYAxis) noexcept;

            void Create(VkExtent2D extent, bool flipYAxis) noexcept;

            void Create(const VkViewport& viewport) noexcept;

            void Update(float width, float height, bool flipYAxis) noexcept;

            void Update(VkExtent2D extent, bool flipYAxis) noexcept;

            void Update(const VkViewport& viewport) noexcept;

            void Set(VkCommandBuffer commandBuffer) noexcept;

          private:
            VkViewport m_Viewport;
        };
    } // namespace vk
} // namespace adh
