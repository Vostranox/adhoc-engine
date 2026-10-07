#pragma once
#include "DeviceQueues.hpp"
#include <Std/Array.hpp>
#include <Utility.hpp>

#include <vulkan/vulkan.h>

namespace adh {
    namespace vk {
        class CommandBuffer {
          public:
            CommandBuffer() noexcept = default;

            CommandBuffer(VkCommandBufferLevel bufferLevel,
                          std::uint32_t count,
                          VkCommandPoolCreateFlagBits poolCreateFlag,
                          DeviceQueues::Family queueFamily) noexcept;

            CommandBuffer(const CommandBuffer& rhs) = delete;

            CommandBuffer& operator=(const CommandBuffer& rhs) = delete;

            CommandBuffer(CommandBuffer&& rhs) noexcept;

            CommandBuffer& operator=(CommandBuffer&& rhs) noexcept;

            ~CommandBuffer() = default;

            void Create(VkCommandBufferLevel bufferLevel,
                        std::uint32_t count,
                        VkCommandPoolCreateFlagBits poolCreateFlag,
                        DeviceQueues::Family queueFamily);

            VkCommandBuffer Begin(std::size_t index = 0u, VkCommandBufferUsageFlagBits usageFlag = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

            void End(std::size_t index = 0u);

            void Reset();

            void Reset(std::size_t index);

            void Free();

            void Free(std::size_t index);

            void Destroy() noexcept;

            VkCommandBuffer& operator[](std::size_t index) noexcept;

            const VkCommandBuffer& operator[](std::size_t index) const noexcept;

          private:
            void MoveConstruct(CommandBuffer&& rhs);

            void Clear() noexcept;

          private:
            Array<VkCommandBuffer> m_CommandBuffers;
            VkCommandPool m_CommandPool{ VK_NULL_HANDLE };
        };
    } // namespace vk
} // namespace adh
