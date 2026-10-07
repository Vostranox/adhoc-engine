#include "Device.hpp"
#include "Initializers.hpp"
#include "Tools.hpp"
#include <Std/Utility.hpp>
#include <set>

namespace adh {
    namespace vk {

        Device::Device() noexcept : m_Device{ VK_NULL_HANDLE } {
        }

        Device::Device(VkPhysicalDevice physicalDevice, DeviceQueues* queues) {
            Create(physicalDevice, queues);
        }

        Device::Device(Device&& rhs) noexcept {
            MoveConstruct(Move(rhs));
        }

        Device& Device::operator=(Device&& rhs) noexcept {
            Clear();
            MoveConstruct(Move(rhs));
            return *this;
        }

        VkBool32 Device::IsRaytracingSupported() const noexcept {
            return m_SupportsRayTracing;
        }

        Device::~Device() {
            Clear();
        }

        void Device::Create(VkPhysicalDevice physicalDevice, DeviceQueues* queues) {
            auto uniqueQueues{ queues->GetUniqueQueuesIDs() };
            Array<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;
            for (std::uint32_t queueFamilyIndex : uniqueQueues) {
                auto queueCreateInfo{ initializers::DeviceQueueCreateInfo(queueFamilyIndex) };
                deviceQueueCreateInfos.EmplaceBack(queueCreateInfo);
            }

            auto physicalDeviceFeatures{ tools::GetPhysicalDeviceFeatures(physicalDevice) };
            Array<const char*> deviceExtentions;
            deviceExtentions.EmplaceBack("VK_KHR_swapchain");
#if defined(ADH_APPLE)
            deviceExtentions.EmplaceBack("VK_KHR_portability_subset");
#endif
            m_SupportsRayTracing = false;
            if (m_SupportsRayTracing) {
                deviceExtentions.EmplaceBack("VK_KHR_ray_tracing_pipeline");
                deviceExtentions.EmplaceBack("VK_KHR_acceleration_structure");
                deviceExtentions.EmplaceBack("VK_KHR_spirv_1_4");
                deviceExtentions.EmplaceBack("VK_KHR_shader_float_controls");
                deviceExtentions.EmplaceBack("VK_KHR_get_memory_requirements2");
                deviceExtentions.EmplaceBack("VK_EXT_descriptor_indexing");
                deviceExtentions.EmplaceBack("VK_KHR_buffer_device_address");
                deviceExtentions.EmplaceBack("VK_KHR_deferred_host_operations");
                deviceExtentions.EmplaceBack("VK_KHR_pipeline_library");
                deviceExtentions.EmplaceBack("VK_KHR_maintenance3");
                deviceExtentions.EmplaceBack("VK_KHR_maintenance1");
            }
            tools::CheckDeviceExtensionAvailability(physicalDevice, deviceExtentions);

            auto createInfo{ initializers::DeviceCreateInfo(deviceExtentions) };
            createInfo.queueCreateInfoCount = static_cast<std::uint32_t>(deviceQueueCreateInfos.GetSize());
            createInfo.pQueueCreateInfos    = deviceQueueCreateInfos.GetData();
            createInfo.pEnabledFeatures     = &physicalDeviceFeatures;

            if (m_SupportsRayTracing) {
                auto bufferDeviceAddressFeatures{ initializers::PhysicalDeviceBufferDeviceAddressFeatures() };
                auto rayTracingPipelineFeatures{ initializers::PhysicalDeviceRayTracingPipelineFeatures(bufferDeviceAddressFeatures) };
                auto accelerationStructureFeatures{ initializers::PhysicalDeviceAccelerationStructureFeatures(rayTracingPipelineFeatures) };
                createInfo.pNext = &accelerationStructureFeatures;
            }

            ADH_THROW(vkCreateDevice(physicalDevice, &createInfo, nullptr, &m_Device) == VK_SUCCESS,
                      "Failed to create device!");
            GetDeviceQueues(queues);
        }

        void Device::Destroy() noexcept {
            Clear();
        }

        Device::operator VkDevice() noexcept {
            return m_Device;
        }

        Device::operator VkDevice() const noexcept {
            return m_Device;
        }

        void Device::GetDeviceQueues(DeviceQueues* queues) noexcept {
            if (queues->graphics.index) {
                vkGetDeviceQueue(m_Device, queues->graphics.index.value(), 0u, &queues->graphics.queue);
            }
            if (queues->compute.index) {
                vkGetDeviceQueue(m_Device, queues->compute.index.value(), 0u, &queues->compute.queue);
            }
            if (queues->transfer.index) {
                vkGetDeviceQueue(m_Device, queues->transfer.index.value(), 0u, &queues->transfer.queue);
            }
            if (queues->sparse.index) {
                vkGetDeviceQueue(m_Device, queues->sparse.index.value(), 0u, &queues->sparse.queue);
            }
            if (queues->present.index) {
                vkGetDeviceQueue(m_Device, queues->present.index.value(), 0u, &queues->present.queue);
            }
        }

        void Device::MoveConstruct(Device&& rhs) noexcept {
            m_Device     = rhs.m_Device;
            rhs.m_Device = VK_NULL_HANDLE;
        }

        void Device::Clear() noexcept {
            if (m_Device != VK_NULL_HANDLE) {
                vkDestroyDevice(m_Device, nullptr);
                m_Device = VK_NULL_HANDLE;
            }
        }
    } // namespace vk
} // namespace adh
