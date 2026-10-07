#pragma once
#include <Std/Array.hpp>
#include <Utility.hpp>

#include <cstring>
#include <iostream>
#include <limits>

#include <vulkan/vulkan.h>

namespace adh {
    namespace vk {
        namespace tools {
            inline auto GetPhysicalDevices(VkInstance instance) {
                std::uint32_t count{};
                vkEnumeratePhysicalDevices(instance, &count, nullptr);
                Array<VkPhysicalDevice> physicalDevices;
                physicalDevices.Resize(count);
                vkEnumeratePhysicalDevices(instance, &count, physicalDevices.GetData());
                return physicalDevices;
            }

            inline auto GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice) noexcept {
                VkPhysicalDeviceFeatures deviceFeatures;
                vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
                return deviceFeatures;
            }

            inline auto GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice) noexcept {
                VkPhysicalDeviceProperties deviceProperties;
                vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
                return deviceProperties;
            }

            inline auto GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice) {
                std::uint32_t count{};
                vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
                Array<VkQueueFamilyProperties> queueFamilies;
                queueFamilies.Resize(count);
                vkGetPhysicalDeviceQueueFamilyProperties(
                    physicalDevice,
                    &count,
                    queueFamilies.GetData());
                return queueFamilies;
            }

            inline auto GetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format) {
                VkFormatProperties properties;
                vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);
                return properties;
            }

            inline auto GetMaxSampleCount(VkPhysicalDevice physicalDevice) {
                auto physicalDeviceProperties{ GetPhysicalDeviceProperties(physicalDevice) };

                VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
                if (counts & VK_SAMPLE_COUNT_64_BIT) {
                    return VK_SAMPLE_COUNT_64_BIT;
                }
                if (counts & VK_SAMPLE_COUNT_32_BIT) {
                    return VK_SAMPLE_COUNT_32_BIT;
                }
                if (counts & VK_SAMPLE_COUNT_16_BIT) {
                    return VK_SAMPLE_COUNT_16_BIT;
                }
                if (counts & VK_SAMPLE_COUNT_8_BIT) {
                    return VK_SAMPLE_COUNT_8_BIT;
                }
                if (counts & VK_SAMPLE_COUNT_4_BIT) {
                    return VK_SAMPLE_COUNT_4_BIT;
                }
                if (counts & VK_SAMPLE_COUNT_2_BIT) {
                    return VK_SAMPLE_COUNT_2_BIT;
                }

                return VK_SAMPLE_COUNT_1_BIT;
            }

            inline auto GetSupportedDepthStencilFormat(VkPhysicalDevice physicalDevice) {
                const VkFormatFeatureFlags features{ VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT };
                for (auto format : { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT }) {
                    if ((GetPhysicalDeviceFormatProperties(physicalDevice, format).optimalTilingFeatures & features) == features) {
                        return format;
                    }
                }
                throw std::runtime_error("Could not find supported depth/stencil format!");
            }

            inline auto GetSupportedShadowMapFormat(VkPhysicalDevice physicalDevice) {
                const VkFormatFeatureFlags features{ VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT };
                for (auto format : { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D16_UNORM }) {
                    if ((GetPhysicalDeviceFormatProperties(physicalDevice, format).optimalTilingFeatures & features) == features) {
                        return format;
                    }
                }
                throw std::runtime_error("Could not find a supported shadow map format!");
            }

            inline auto GetDeviceExtensionProperties(VkPhysicalDevice physicalDevice) {
                std::uint32_t count{};
                vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);
                Array<VkExtensionProperties> deviceExtensionProperties;
                deviceExtensionProperties.Resize(count);
                vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, deviceExtensionProperties.GetData());
                return deviceExtensionProperties;
            }

            inline auto GetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice) {
                VkPhysicalDeviceMemoryProperties memoryProperties;
                vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
                return memoryProperties;
            }

            inline auto GetSurfaceCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) noexcept {
                VkSurfaceCapabilitiesKHR surfaceCapabilities{};
                vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                    physicalDevice,
                    surface,
                    &surfaceCapabilities);
                return surfaceCapabilities;
            }

            inline bool CheckForExtentionSupport(VkPhysicalDevice physicalDevice, const char* extention) {
                auto deviceExtensionsProperties{ GetDeviceExtensionProperties(physicalDevice) };
                bool found{};
                for (std::size_t j{}; j != deviceExtensionsProperties.GetSize(); ++j) {
                    if (!strcmp(extention, deviceExtensionsProperties[j].extensionName)) {
                        found = true;
                        break;
                    }
                }
                if (found) {
                    return true;
                } else {
                    return false;
                }
            }

            inline void CheckDeviceExtensionAvailability(VkPhysicalDevice physicalDevice, const Array<const char*>& deviceExtentions) {
                auto deviceExtensionsProperties{ GetDeviceExtensionProperties(physicalDevice) };
                for (std::size_t i{}; i != deviceExtentions.GetSize(); ++i) {
                    bool found{};
                    for (std::size_t j{}; j != deviceExtensionsProperties.GetSize(); ++j) {
                        if (!strcmp(deviceExtentions[i], deviceExtensionsProperties[j].extensionName)) {
                            found = true;
                            break;
                        }
                    }
                    ADH_THROW(found, "Requested unavailable device extension!");
                }
            }

            inline auto GetSwapchainImages(VkDevice device, VkSwapchainKHR swapchain) {
                std::uint32_t count{};
                vkGetSwapchainImagesKHR(device, swapchain, &count, nullptr);
                Array<VkImage> images;
                images.Resize(count);
                vkGetSwapchainImagesKHR(device, swapchain, &count, images.GetData());
                return images;
            }

            inline auto GetPhysicalDeviceSurfaceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
                std::uint32_t count;
                vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, nullptr);
                Array<VkSurfaceFormatKHR> surfaceFormats;
                surfaceFormats.Resize(count);
                vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, surfaceFormats.GetData());
                return surfaceFormats;
            }

            inline auto GetPhysicalDeviceSurfacePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
                std::uint32_t count;
                vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, nullptr);
                Array<VkPresentModeKHR> presentModes;
                presentModes.Resize(count);
                vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, presentModes.GetData());
                return presentModes;
            }

            inline auto GetBufferMemoryRequirements(VkDevice device, VkBuffer buffer) noexcept {
                VkMemoryRequirements memoryRequirements;
                vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);
                return memoryRequirements;
            }

            inline auto GetImageMemoryRequirements(VkDevice device, VkImage image) noexcept {
                VkMemoryRequirements memoryRequirements;
                vkGetImageMemoryRequirements(device, image, &memoryRequirements);
                return memoryRequirements;
            }

            inline std::uint32_t GetMemoryTypeIndex(
                VkPhysicalDevice physicalDevice,
                VkMemoryRequirements memoryRequirements,
                VkMemoryPropertyFlagBits memoryPropertyFlag) ADH_NOEXCEPT {
                auto memoryProperties{ GetPhysicalDeviceMemoryProperties(physicalDevice) };
                constexpr std::uint32_t invalidIndex{ std::numeric_limits<std::uint32_t>::max() };
                std::uint32_t memoryTypeIndex{ invalidIndex };
                for (std::uint32_t i{}; i != memoryProperties.memoryTypeCount; ++i) {
                    if ((memoryRequirements.memoryTypeBits & (1u << i)) &&
                        (memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlag) == memoryPropertyFlag) {
                        memoryTypeIndex = i;
                        break;
                    }
                }
                ADH_THROW(memoryTypeIndex != invalidIndex, "Could not find compatible memory index type!");
                return memoryTypeIndex;
            }

            inline bool IsUniformMemoryAccess(VkPhysicalDevice physicalDevice) {
                auto properties{ GetPhysicalDeviceMemoryProperties(physicalDevice) };
                if (properties.memoryHeapCount == 1 && properties.memoryHeaps[0].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                    return true;
                } else {
                    return false;
                }
            }

        } // namespace tools
    }     // namespace vk
} // namespace adh
