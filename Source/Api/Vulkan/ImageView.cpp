#include <Vulkan/Context.hpp>
#include <Vulkan/Initializers.hpp>

#include "ImageView.hpp"

namespace adh {
    namespace vk {
        ImageView::ImageView(VkImage image,
                             VkImageViewType viewType,
                             VkFormat format,
                             VkImageAspectFlagBits imageAspectFlags,
                             std::uint32_t levelCount,
                             std::uint32_t layerCount,
                             std::uint32_t baseArrayLayer) ADH_NOEXCEPT {
            Create(image, viewType, format, imageAspectFlags, levelCount, layerCount, baseArrayLayer);
        }

        ImageView::ImageView(ImageView&& rhs) noexcept {
            MoveConstruct(Move(rhs));
        }

        ImageView& ImageView::operator=(ImageView&& rhs) noexcept {
            Clear();
            MoveConstruct(Move(rhs));
            return *this;
        }

        ImageView::~ImageView() {
            Clear();
        }

        void ImageView::Create(VkImage image,
                               VkImageViewType viewType,
                               VkFormat format,
                               VkImageAspectFlagBits imageAspectFlags,
                               std::uint32_t levelCount,
                               std::uint32_t layerCount,
                               std::uint32_t baseArrayLayer) ADH_NOEXCEPT {
            auto info{ initializers::ImageViewCreateInfo(image, viewType, format, imageAspectFlags, levelCount, layerCount) };
            info.subresourceRange.baseArrayLayer = baseArrayLayer;

            ADH_THROW(vkCreateImageView(Context::Get()->GetDevice(), &info, nullptr, &mImageView) == VK_SUCCESS,
                      "Failed to create image view!");
        }

        void ImageView::Destroy() {
            Clear();
        }

        VkImageView& ImageView::Get() noexcept {
            return mImageView;
        }

        const VkImageView& ImageView::Get() const noexcept {
            return mImageView;
        }

        ImageView::operator VkImageView() noexcept {
            return mImageView;
        }

        ImageView::operator VkImageView() const noexcept {
            return mImageView;
        }

        void ImageView::MoveConstruct(ImageView&& rhs) noexcept {
            mImageView     = rhs.mImageView;
            rhs.mImageView = VK_NULL_HANDLE;
        }

        void ImageView::Clear() noexcept {
            if (mImageView != VK_NULL_HANDLE) {
                vkDestroyImageView(Context::Get()->GetDevice(), mImageView, nullptr);
                mImageView = VK_NULL_HANDLE;
            }
        }
    } // namespace vk
} // namespace adh
