#pragma once
#include <vulkan/vulkan.h>

namespace adh {
    namespace vk {
        class PhysicalDevice {
          public:
            PhysicalDevice() noexcept;

            PhysicalDevice(VkInstance instance);

            void Create(VkInstance instance);

            operator VkPhysicalDevice() noexcept;

            operator VkPhysicalDevice() const noexcept;

          private:
            VkPhysicalDevice m_PhysicalDevice;
        };
    } // namespace vk
} // namespace adh
