#include "Scissor.hpp"
#include "Initializers.hpp"

namespace adh {
    namespace vk {
        Scissor::Scissor(std::uint32_t width, std::uint32_t height, std::int32_t offsetX, std::int32_t offsetY) noexcept {
            Create(width, height, offsetX, offsetY);
        }

        Scissor::Scissor(VkExtent2D extent, VkOffset2D offset) noexcept {
            Create(extent, offset);
        }

        Scissor::Scissor(const VkRect2D& rect) noexcept {
            m_Scissor = rect;
        }

        void Scissor::Create(std::uint32_t width, std::uint32_t height, std::int32_t offsetX, std::int32_t offsetY) noexcept {
            m_Scissor = initializers::Rect2D({ width, height }, { offsetX, offsetY });
        }

        void Scissor::Create(VkExtent2D extent, VkOffset2D offset) noexcept {
            m_Scissor = initializers::Rect2D(extent, offset);
        }

        void Scissor::Create(const VkRect2D& rect) noexcept {
            m_Scissor = rect;
        }

        void Scissor::Update(std::uint32_t width, std::uint32_t height, std::int32_t offsetX, std::int32_t offsetY) noexcept {
            Create(width, height, offsetX, offsetY);
        }

        void Scissor::Update(VkExtent2D extent, VkOffset2D offset) noexcept {
            Create(extent, offset);
        }

        void Scissor::Update(const VkRect2D& rect) noexcept {
            Create(rect);
        }

        void Scissor::Set(VkCommandBuffer commandBuffer) noexcept {
            vkCmdSetScissor(commandBuffer, 0, 1, &m_Scissor);
        }
    } // namespace vk
} // namespace adh
