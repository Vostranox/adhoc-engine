#pragma once
#include "DeviceQueues.hpp"
#include <vulkan/vulkan.h>

namespace adh {
    namespace vk {
        class Device {
          public:
            Device() noexcept;

            Device(VkPhysicalDevice physicalDevice, DeviceQueues* queues);

            Device(const Device&) = delete;

            Device& operator=(const Device&) = delete;

            Device(Device&& rhs) noexcept;

            Device& operator=(Device&& rhs) noexcept;

            ~Device();

            void Create(VkPhysicalDevice physicalDevice, DeviceQueues* queues);

            VkBool32 IsRaytracingSupported() const noexcept;

            void Destroy() noexcept;

            operator VkDevice() noexcept;

            operator VkDevice() const noexcept;

          private:
            void GetDeviceQueues(DeviceQueues* queues) noexcept;

            void MoveConstruct(Device&& rhs) noexcept;

            void Clear() noexcept;

          private:
            VkDevice m_Device;
            VkBool32 m_SupportsRayTracing;
        };
    } // namespace vk
} // namespace adh
