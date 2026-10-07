#pragma once
#include "Allocator.hpp"
#include <Utility.hpp>
#include <vulkan/vulkan.h>

namespace adh {
    namespace vk {
        class Image {
          public:
            Image() noexcept = default;

            Image(VkExtent3D extent,
                  VkFormat format,
                  VkImageTiling imageTiling,
                  VkImageType imageType,
                  VkImageCreateFlagBits imageCreateFlags,
                  std::uint32_t mipLevels,
                  std::uint32_t arrayLayers,
                  VkSampleCountFlagBits samples,
                  VkImageUsageFlagBits imageUsageFlag,
                  VkImageAspectFlagBits imageAspectFlag,
                  VkImageViewType imageViewType,
                  VkMemoryPropertyFlagBits memoryProperty,
                  VkSharingMode sharingMode);

            Image(const Image&) = delete;

            Image& operator=(const Image&) = delete;

            Image(Image&& rhs) noexcept;

            Image& operator=(Image&& rhs) noexcept;

            ~Image();

            void Create(VkExtent3D extent,
                        VkFormat format,
                        VkImageTiling imageTiling,
                        VkImageType imageType,
                        VkImageCreateFlagBits imageCreateFlags,
                        std::uint32_t mipLevels,
                        std::uint32_t arrayLayers,
                        VkSampleCountFlagBits samples,
                        VkImageUsageFlagBits imageUsageFlag,
                        VkImageAspectFlagBits imageAspectFlag,
                        VkImageViewType imageViewType,
                        VkMemoryPropertyFlagBits memoryProperty,
                        VkSharingMode sharingMode);

            void Bind() ADH_NOEXCEPT;

            void Destroy() noexcept;

            VkImage GetImage() noexcept;

            VkImage GetImage() const noexcept;

            VkImageView GetImageView() noexcept;

            VkImageView GetImageView() const noexcept;

            operator VkImage() noexcept;

            operator VkImage() const noexcept;

          private:
            void MoveConstruct(Image&& rhs) noexcept;

            void Clear() noexcept;

          private:
            ImageData m_Image;
        };
    } // namespace vk
} // namespace adh
