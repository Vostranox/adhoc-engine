#pragma once
#include <iostream>
#include <vulkan/vulkan.h>

#include <Utility.hpp>

namespace adh {
    namespace vk {
        class ImageView {
          public:
            ImageView() = default;

            ImageView(VkImage image,
                      VkImageViewType viewType,
                      VkFormat format,
                      VkImageAspectFlagBits imageAspectFlags,
                      std::uint32_t levelCount,
                      std::uint32_t layerCount,
                      std::uint32_t baseArrayLayer) ADH_NOEXCEPT;

            ImageView(const ImageView& rhs) = delete;

            ImageView& operator=(const ImageView& rhs) = delete;

            ImageView(ImageView&& rhs) noexcept;

            ImageView& operator=(ImageView&& rhs) noexcept;

            ~ImageView();

            void Create(VkImage image,
                        VkImageViewType viewType,
                        VkFormat format,
                        VkImageAspectFlagBits imageAspectFlags,
                        std::uint32_t levelCount,
                        std::uint32_t layerCount,
                        std::uint32_t baseArrayLayer) ADH_NOEXCEPT;

            void Destroy();

            VkImageView& Get() noexcept;

            const VkImageView& Get() const noexcept;

            operator VkImageView() noexcept;

            operator VkImageView() const noexcept;

          private:
            void MoveConstruct(ImageView&& rhs) noexcept;

            void Clear() noexcept;

          private:
            VkImageView mImageView{ VK_NULL_HANDLE };
        };
    } // namespace vk
} // namespace adh
