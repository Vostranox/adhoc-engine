#pragma once
#include <Utility.hpp>
#include <set>
#include <thread>
#include <unordered_map>
#include <vulkan/vulkan.h>

namespace adh {
    namespace vk {
        namespace _internal {
            struct CommandPoolData {
                VkCommandPoolCreateFlagBits flag;
                std::uint32_t queueIndex;
                mutable VkCommandPool commandPool{ VK_NULL_HANDLE };
            };

            constexpr bool operator<(const CommandPoolData& lhs, const CommandPoolData& rhs) {
                return (lhs.flag < rhs.flag) || (lhs.flag == rhs.flag && lhs.queueIndex < rhs.queueIndex);
            }
        } // namespace _internal
    }     // namespace vk
} // namespace adh

namespace adh {
    namespace vk {
        class CommandPool {
          public:
            CommandPool();

            CommandPool(const CommandPool& rhs) = delete;

            CommandPool& operator=(const CommandPool& rhs) = delete;

            CommandPool(CommandPool&& rhs) = delete;

            CommandPool& operator=(CommandPool&& rhs) = delete;

            ~CommandPool();

            VkCommandPool Get(VkCommandPoolCreateFlagBits flag, std::uint32_t queueIndex);

            void Reset(VkCommandPoolCreateFlagBits flag, std::uint32_t queueIndex) ADH_NOEXCEPT;

            void Destroy() noexcept;

          private:
            void Clear() noexcept;

          public:
            static CommandPool* Get() noexcept;

          private:
            std::set<_internal::CommandPoolData> m_CommandPools;

          private:
            inline static std::unordered_map<std::thread::id, CommandPool*> m_Pools;
        };
    } // namespace vk
} // namespace adh
