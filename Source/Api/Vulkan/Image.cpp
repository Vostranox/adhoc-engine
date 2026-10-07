#include "Image.hpp"

namespace adh {
    namespace vk {
        Image::Image(VkExtent3D extent,
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
                     VkSharingMode sharingMode) {
            Create(extent,
                   format,
                   imageTiling,
                   imageType,
                   imageCreateFlags,
                   mipLevels,
                   arrayLayers,
                   samples,
                   imageUsageFlag,
                   imageAspectFlag,
                   imageViewType,
                   memoryProperty,
                   sharingMode);
        }

        Image::Image(Image&& rhs) noexcept {
            MoveConstruct(Move(rhs));
        }

        Image& Image::operator=(Image&& rhs) noexcept {
            Clear();
            MoveConstruct(Move(rhs));

            return *this;
        }

        Image::~Image() {
            Clear();
        }

        void Image::Create(VkExtent3D extent,
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
                           VkSharingMode sharingMode) {
            ADH_THROW(m_Image.image == VK_NULL_HANDLE, "Calling create on an already created image!");
            Allocator::CreateImage({ extent.width, extent.height, extent.depth },
                                   format,
                                   imageTiling,
                                   imageType,
                                   imageCreateFlags,
                                   mipLevels,
                                   arrayLayers,
                                   samples,
                                   imageUsageFlag,
                                   memoryProperty,
                                   sharingMode,
                                   m_Image);

            Bind();

            Allocator::CreateImageView(
                imageViewType,
                format,
                imageAspectFlag,
                mipLevels,
                arrayLayers,
                m_Image);
        }

        void Image::Bind() ADH_NOEXCEPT {
            Allocator::BindImage(m_Image);
        }

        void Image::Destroy() noexcept {
            Clear();
        }

        VkImage Image::GetImage() noexcept {
            return m_Image.image;
        }

        VkImage Image::GetImage() const noexcept {
            return m_Image.image;
        }

        VkImageView Image::GetImageView() noexcept {
            return m_Image.imageView;
        }

        VkImageView Image::GetImageView() const noexcept {
            return m_Image.imageView;
        }

        Image::operator VkImage() noexcept {
            return m_Image.image;
        }

        Image::operator VkImage() const noexcept {
            return m_Image.image;
        }

        void Image::MoveConstruct(Image&& rhs) noexcept {
            m_Image     = rhs.m_Image;
            rhs.m_Image = {};
        }

        void Image::Clear() noexcept {
            if (m_Image.image != VK_NULL_HANDLE &&
                m_Image.imageView != VK_NULL_HANDLE) {
                Allocator::DestroyImage(m_Image);
                m_Image = {};
            }
        }
    } // namespace vk
} // namespace adh
