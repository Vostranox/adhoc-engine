#pragma once

#include <Utility.hpp>
#include <iostream>
#include <vulkan/vulkan.h>

namespace adh {
    namespace vk {
        class Framebuffer {
          public:
            Framebuffer() = default;

            Framebuffer(VkRenderPass renderPass,
                        std::uint32_t attachmentCount,
                        const VkImageView* attachments,
                        VkExtent2D extent,
                        std::uint32_t layers);

            Framebuffer(const Framebuffer& rhs) = delete;

            Framebuffer& operator=(const Framebuffer& rhs) = delete;

            Framebuffer(Framebuffer&& rhs) noexcept;

            Framebuffer& operator=(Framebuffer&& rhs) noexcept;

            ~Framebuffer();

            void Create(VkRenderPass renderPass,
                        std::uint32_t attachmentCount,
                        const VkImageView* attachments,
                        VkExtent2D extent,
                        std::uint32_t layers) ADH_NOEXCEPT;

            void Destroy();

            VkFramebuffer& Get() noexcept;

            const VkFramebuffer& Get() const noexcept;

            operator VkFramebuffer() noexcept;

            operator VkFramebuffer() const noexcept;

          private:
            void MoveConstruct(Framebuffer&& rhs) noexcept;

            void Clear() noexcept;

          private:
            VkFramebuffer mFramebuffer{ VK_NULL_HANDLE };
        };
    } // namespace vk
} // namespace adh
