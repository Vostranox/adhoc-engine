#include "DeviceQueues.hpp"
#include "Tools.hpp"

namespace adh {
    namespace vk {
        DeviceQueues::DeviceQueues() noexcept = default;

        DeviceQueues::DeviceQueues(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
            Create(physicalDevice, surface);
        }

        void DeviceQueues::Create(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
            auto queueFamilies{ tools::GetPhysicalDeviceQueueFamilyProperties(physicalDevice) };

            for (std::uint32_t i{}; i != queueFamilies.GetSize(); ++i) {
                if (!graphics.index &&
                    queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    graphics.index = static_cast<std::int32_t>(i);
                }

                if (!compute.index &&
                    queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
                    compute.index = static_cast<std::int32_t>(i);
                }

                if (!transfer.index &&
                    queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
                    transfer.index = static_cast<std::int32_t>(i);
                }

                if (!sparse.index &&
                    queueFamilies[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
                    sparse.index = static_cast<std::int32_t>(i);
                }

                VkBool32 presentSupport{};
                ADH_THROW(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport) == VK_SUCCESS,
                          "Failed to get physical device surface support!");

                if (!present.index && presentSupport) {
                    present.index = static_cast<std::int32_t>(i);
                }
            }
        }

        std::set<std::uint32_t> DeviceQueues::GetUniqueQueuesIDs() const {
            std::set<std::uint32_t> uniqueFamilies;
            if (graphics.index.has_value()) {
                uniqueFamilies.emplace(graphics.index.value());
            }
            if (compute.index.has_value()) {
                uniqueFamilies.emplace(compute.index.value());
            }
            if (transfer.index.has_value()) {
                uniqueFamilies.emplace(transfer.index.value());
            }
            if (sparse.index.has_value()) {
                uniqueFamilies.emplace(sparse.index.value());
            }
            if (present.index.has_value()) {
                uniqueFamilies.emplace(present.index.value());
            }
            return uniqueFamilies;
        }

        DeviceQueueData DeviceQueues::operator[](Family index) const noexcept {
            switch (index) {
            case eGraphics:
                return graphics;
            case eCompute:
                return compute;
            case eTransfer:
                return transfer;
            case eSparse:
                return sparse;
            case ePrensent:
                return present;
            }
            return {};
        }
    } // namespace vk
} // namespace adh
