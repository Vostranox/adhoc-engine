#include "CommandBuffer.hpp"
#include "CommandPool.hpp"
#include "Context.hpp"
#include "Initializers.hpp"
#include <Utility.hpp>

namespace adh {
    namespace vk {
        CommandBuffer::CommandBuffer(VkCommandBufferLevel bufferLevel,
                                     std::uint32_t count,
                                     VkCommandPoolCreateFlagBits poolCreateFlag,
                                     DeviceQueues::Family queueFamily) noexcept {
            Create(bufferLevel, count, poolCreateFlag, queueFamily);
        }

        CommandBuffer::CommandBuffer(CommandBuffer&& rhs) noexcept {
            MoveConstruct(Move(rhs));
        }

        CommandBuffer& CommandBuffer::operator=(CommandBuffer&& rhs) noexcept {
            Clear();
            MoveConstruct(Move(rhs));
            return *this;
        }

        void CommandBuffer::Create(VkCommandBufferLevel bufferLevel,
                                   std::uint32_t count,
                                   VkCommandPoolCreateFlagBits poolCreateFlag,
                                   DeviceQueues::Family queueFamily) {
            m_CommandPool = CommandPool::Get()->Get(poolCreateFlag, Context::Get()->GetQueue(queueFamily).index.value());
            auto info{ initializers::CommandBufferAllocateInfo(m_CommandPool, count, bufferLevel) };
            m_CommandBuffers.Resize(count);
            ADH_THROW(vkAllocateCommandBuffers(Context::Get()->GetDevice(), &info, m_CommandBuffers.GetData()) == VK_SUCCESS,
                      "Failed to allocate command buffers!");
        }

        VkCommandBuffer CommandBuffer::Begin(std::size_t index, VkCommandBufferUsageFlagBits usageFlag) {
            auto info{ initializers::CommandBufferBeginInfo(usageFlag) };
            ADH_THROW(vkBeginCommandBuffer(m_CommandBuffers[index], &info) == VK_SUCCESS,
                      "Failed to begin command buffer!");
            return m_CommandBuffers[index];
        }

        void CommandBuffer::End(std::size_t index) {
            ADH_THROW(vkEndCommandBuffer(m_CommandBuffers[index]) == VK_SUCCESS,
                      "Failed to end command buffer!");
        }

        void CommandBuffer::Reset() {
            for (std::size_t i{}; i != m_CommandBuffers.GetSize(); ++i) {
                ADH_THROW(vkResetCommandBuffer(m_CommandBuffers[i], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT),
                          "Failed to reset command buffer!");
            }
        }

        void CommandBuffer::Reset(std::size_t index) {
            ADH_THROW(vkResetCommandBuffer(m_CommandBuffers[index], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT),
                      "Failed to reset command buffer!");
        }

        void CommandBuffer::Free() {
            vkFreeCommandBuffers(Context::Get()->GetDevice(), m_CommandPool, static_cast<std::uint32_t>(m_CommandBuffers.GetSize()), m_CommandBuffers.GetData());
        }

        void CommandBuffer::Free(std::size_t index) {
            vkFreeCommandBuffers(Context::Get()->GetDevice(), m_CommandPool, 1, &m_CommandBuffers[index]);
        }

        void CommandBuffer::Destroy() noexcept {
            Clear();
        }

        VkCommandBuffer& CommandBuffer::operator[](std::size_t index) noexcept {
            return m_CommandBuffers[index];
        }

        const VkCommandBuffer& CommandBuffer::operator[](std::size_t index) const noexcept {
            return m_CommandBuffers[index];
        }

        void CommandBuffer::MoveConstruct(CommandBuffer&& rhs) {
            m_CommandBuffers = Move(rhs.m_CommandBuffers);
            m_CommandPool    = rhs.m_CommandPool;
        }

        void CommandBuffer::Clear() noexcept {
            if (!m_CommandBuffers.IsEmpty()) {
                m_CommandBuffers.Clear();
                m_CommandPool = VK_NULL_HANDLE;
            }
        }
    } // namespace vk
} // namespace adh
