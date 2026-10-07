#include "Framebuffer.hpp"
#include <Vulkan/Context.hpp>
#include <Vulkan/Initializers.hpp>

namespace adh {
    namespace vk {
        Framebuffer::Framebuffer(VkRenderPass renderPass,
                                 std::uint32_t attachmentCount,
                                 const VkImageView* attachments,
                                 VkExtent2D extent,
                                 std::uint32_t layers) {
            Create(renderPass, attachmentCount, attachments, extent, layers);
        }

        Framebuffer::Framebuffer(Framebuffer&& rhs) noexcept {
            MoveConstruct(Move(rhs));
        }

        Framebuffer& Framebuffer::operator=(Framebuffer&& rhs) noexcept {
            Clear();
            MoveConstruct(Move(rhs));
            return *this;
        }

        Framebuffer::~Framebuffer() {
            Clear();
        }

        void Framebuffer::Create(VkRenderPass renderPass,
                                 std::uint32_t attachmentCount,
                                 const VkImageView* attachments,
                                 VkExtent2D extent,
                                 std::uint32_t layers) ADH_NOEXCEPT {
            auto info{ initializers::FramebufferCreateInfo(renderPass, attachmentCount, attachments, extent, layers) };
            ADH_THROW(vkCreateFramebuffer(Context::Get()->GetDevice(), &info, nullptr, &mFramebuffer) == VK_SUCCESS,
                      "Failed to create frame buffers!");
        }

        void Framebuffer::Destroy() {
            Clear();
        }

        VkFramebuffer& Framebuffer::Get() noexcept {
            return mFramebuffer;
        }

        const VkFramebuffer& Framebuffer::Get() const noexcept {
            return mFramebuffer;
        }

        Framebuffer::operator VkFramebuffer() noexcept {
            return mFramebuffer;
        }

        Framebuffer::operator VkFramebuffer() const noexcept {
            return mFramebuffer;
        }

        void Framebuffer::MoveConstruct(Framebuffer&& rhs) noexcept {
            mFramebuffer     = rhs.mFramebuffer;
            rhs.mFramebuffer = VK_NULL_HANDLE;
        }

        void Framebuffer::Clear() noexcept {
            if (mFramebuffer != VK_NULL_HANDLE) {
                vkDestroyFramebuffer(Context::Get()->GetDevice(), mFramebuffer, nullptr);
                mFramebuffer = VK_NULL_HANDLE;
            }
        }

    } // namespace vk

} // namespace adh
