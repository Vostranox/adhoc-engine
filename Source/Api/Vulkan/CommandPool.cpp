#include "CommandPool.hpp"
#include "Context.hpp"
#include "Initializers.hpp"

namespace adh {
    namespace vk {
        CommandPool::CommandPool() {
            m_Pools.emplace(std::this_thread::get_id(), this);
        }

        CommandPool::~CommandPool() {
            Clear();
        }

        VkCommandPool CommandPool::Get(VkCommandPoolCreateFlagBits flag, std::uint32_t queueIndex) {
            const auto& pool{ m_CommandPools.emplace(_internal::CommandPoolData{ flag, queueIndex }) };
            if (pool.second) {
                auto createInfo{ initializers::CommandPoolCreateInfo(flag, queueIndex) };
                ADH_THROW(vkCreateCommandPool(Context::Get()->GetDevice(), &createInfo, nullptr, &pool.first->commandPool) == VK_SUCCESS,
                          "Failed to create command pool!");
            }
            return pool.first->commandPool;
        }

        void CommandPool::Reset(VkCommandPoolCreateFlagBits flag, std::uint32_t queueIndex) ADH_NOEXCEPT {
            auto itr{ m_CommandPools.find({ flag, queueIndex }) };
            if (itr->commandPool != VK_NULL_HANDLE) {
                ADH_THROW(vkResetCommandPool(Context::Get()->GetDevice(), itr->commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT) == VK_SUCCESS,
                          "Failed to reset command poo!");
            }
        }

        void CommandPool::Destroy() noexcept {
            Clear();
        }

        CommandPool* CommandPool::Get() noexcept {
            return m_Pools.find(std::this_thread::get_id())->second;
        }

        void CommandPool::Clear() noexcept {
            auto device{ Context::Get()->GetDevice() };
            vkDeviceWaitIdle(device);
            for (auto&& i : m_CommandPools) {
                vkDestroyCommandPool(device, i.commandPool, nullptr);
                i.commandPool = VK_NULL_HANDLE;
            }
        }
    } // namespace vk
} // namespace adh
