#include "PhysicalDevice.hpp"
#include "Tools.hpp"

#include <Utility.hpp>

namespace adh {
    namespace vk {
        PhysicalDevice::PhysicalDevice() noexcept : m_PhysicalDevice{ VK_NULL_HANDLE } {
        }

        PhysicalDevice::PhysicalDevice(VkInstance instance) {
            Create(instance);
        }

        void PhysicalDevice::Create(VkInstance instance) {
            VkBool32 physicalDeviceFound{};
            ADH_THROW(instance != VK_NULL_HANDLE, "Instance is null!");
            auto physicalDevices{ tools::GetPhysicalDevices(instance) };
            ADH_THROW(physicalDevices.GetSize() != 0u, "Could not find GPU with Vulkan support!");
            for (std::uint32_t i{}; i != physicalDevices.GetSize(); ++i) {
                auto deviceProperties{ tools::GetPhysicalDeviceProperties(physicalDevices[i]) };
                if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                    m_PhysicalDevice    = physicalDevices[i];
                    physicalDeviceFound = VK_TRUE;
                    break;
                }
            }

            if (!physicalDeviceFound) {
                m_PhysicalDevice = physicalDevices[0];
            }
        }

        PhysicalDevice::operator VkPhysicalDevice() noexcept {
            return m_PhysicalDevice;
        }

        PhysicalDevice::operator VkPhysicalDevice() const noexcept {
            return m_PhysicalDevice;
        }
    } // namespace vk
} // namespace adh
