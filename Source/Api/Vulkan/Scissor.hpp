#pragma once
#include <iostream>
#include <vulkan/vulkan.h>

namespace adh {
    namespace vk {
        class Scissor {
          public:
            Scissor() noexcept = default;

            Scissor(std::uint32_t width, std::uint32_t height, std::int32_t offsetX = 0, std::int32_t offsetY = 0) noexcept;

            Scissor(VkExtent2D extent, VkOffset2D offset = {}) noexcept;

            Scissor(const VkRect2D& rect) noexcept;

            void Create(std::uint32_t width, std::uint32_t height, std::int32_t offsetX = 0, std::int32_t offsetY = 0) noexcept;

            void Create(VkExtent2D extent, VkOffset2D offset = {}) noexcept;

            void Create(const VkRect2D& rect) noexcept;

            void Update(std::uint32_t width, std::uint32_t height, std::int32_t offsetX = 0, std::int32_t offsetY = 0) noexcept;

            void Update(VkExtent2D extent, VkOffset2D offset = {}) noexcept;

            void Update(const VkRect2D& rect) noexcept;

            void Set(VkCommandBuffer commandBuffer) noexcept;

          private:
            VkRect2D m_Scissor;
        };
    } // namespace vk
} // namespace adh
