#pragma once
#include <Std/Array.hpp>
#include <cstdint>
#include <vulkan/vulkan.h>

namespace adh {
    namespace vk {
        class Attachment {
          public:
            enum class Type {
                eColor,
                eDepth,
                eResolve,
                eInput
            };

          public:
            void AddDescription(VkFormat format,
                                VkSampleCountFlagBits samplerCount,
                                VkAttachmentLoadOp loadOp,
                                VkAttachmentStoreOp storeOp,
                                VkAttachmentLoadOp stencilLoadOp,
                                VkAttachmentStoreOp stencilStoreOp,
                                VkImageLayout finalLayout,
                                VkImageLayout attachment,
                                Type type,
                                VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED);

            void AddInputReference(VkImageLayout attachment, std::uint32_t index);

            Array<VkAttachmentDescription>& GetDescriptions() noexcept;

            const Array<VkAttachmentDescription>& GetDescriptions() const noexcept;

            Array<VkAttachmentReference>& GetColorReferences() noexcept;

            const Array<VkAttachmentReference>& GetColorReferences() const noexcept;

            Array<VkAttachmentReference>& GetDepthReferences() noexcept;

            const Array<VkAttachmentReference>& GetDepthReferences() const noexcept;

            Array<VkAttachmentReference>& GetResolveReferences() noexcept;

            const Array<VkAttachmentReference>& GetResolveReferences() const noexcept;

            Array<VkAttachmentReference>& GetInputReferences() noexcept;

            const Array<VkAttachmentReference>& GetInputReferences() const noexcept;

          private:
            Array<VkAttachmentDescription> m_Descriptions;
            Array<VkAttachmentReference> m_ColorReferences;
            Array<VkAttachmentReference> m_DepthReferences;
            Array<VkAttachmentReference> m_ResolveReferences;
            Array<VkAttachmentReference> m_InputReferences;
        };
    } // namespace vk
} // namespace adh
