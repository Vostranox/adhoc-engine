#pragma once
#include <Utility.hpp>

#include <optional>
#include <set>
#include <vulkan/vulkan.h>

namespace adh {
    namespace vk {
        struct DeviceQueueData {
            VkQueue queue;
            std::optional<std::uint32_t> index;
        };

        class DeviceQueues {
          public:
            enum Family {
                eGraphics,
                eCompute,
                eTransfer,
                eSparse,
                ePrensent
            };

          public:
            DeviceQueues() noexcept;

            DeviceQueues(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

            void Create(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

            std::set<std::uint32_t> GetUniqueQueuesIDs() const;

            DeviceQueueData operator[](Family index) const noexcept;

          public:
            DeviceQueueData graphics{};
            DeviceQueueData compute{};
            DeviceQueueData transfer{};
            DeviceQueueData sparse{};
            DeviceQueueData present{};
        };
    } // namespace vk
} // namespace adh
