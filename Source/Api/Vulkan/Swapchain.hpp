#pragma once
#include "Image.hpp"
#include <Std/Array.hpp>

#include <iostream>
#include <vulkan/vulkan.h>

namespace adh {
    namespace vk {
        class Swapchain {
          public:
          public:
            Swapchain() noexcept;

            Swapchain(std::uint32_t imageBuffersCount, VkFormat format, VkPresentModeKHR presentMode);

            Swapchain(const Swapchain&) = delete;

            Swapchain& operator=(const Swapchain&) = delete;

            Swapchain(Swapchain&& rhs) noexcept;

            Swapchain& operator=(Swapchain&& rhs) noexcept;

            ~Swapchain();

            void Create(std::uint32_t imageBuffersCount, VkFormat format, VkPresentModeKHR presentMode);

            void Destroy() noexcept;

            VkSwapchainKHR Get() noexcept;

            VkSwapchainKHR Get() const noexcept;

            Array<VkImageView>& GetImageView() noexcept;

            const Array<VkImageView>& GetImageView() const noexcept;

            VkFormat GetFormat() const noexcept;

            VkExtent2D GetExtent() const noexcept;

            Image& GetColorBuffer() noexcept;

            std::uint32_t GetImageViewCount() const noexcept;

            operator VkSwapchainKHR() noexcept;

            operator VkSwapchainKHR() const noexcept;

          private:
            void FindFormat(VkFormat format, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

            void FindPresentMode(VkPresentModeKHR presentMode, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

            void CreateSwapchain(VkSwapchainKHR oldSwapchain);

            void CreateImageViews();

            void MoveConstruct(Swapchain&& rhs) noexcept;

            void Clear();

          public:
            bool isValid{ false };

          private:
            VkSwapchainKHR m_Swapchain;
            VkSurfaceFormatKHR m_Format;
            VkPresentModeKHR m_PresentMode;
            VkExtent2D m_Extent;
            Array<VkImage> m_Images;
            Array<VkImageView> m_ImageViews;
            Image m_ColorBuffer;
            std::uint32_t m_ImageBuffersCount;
        };
    } // namespace vk
} // namespace adh
