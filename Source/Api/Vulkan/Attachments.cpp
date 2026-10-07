#include "Attachments.hpp"
#include "Context.hpp"
#include "Initializers.hpp"

namespace adh {
    namespace vk {
        void Attachment::AddDescription(VkFormat format,
                                        VkSampleCountFlagBits samplerCount,
                                        VkAttachmentLoadOp loadOp,
                                        VkAttachmentStoreOp storeOp,
                                        VkAttachmentLoadOp stencilLoadOp,
                                        VkAttachmentStoreOp stencilStoreOp,
                                        VkImageLayout finalLayout,
                                        VkImageLayout attachment,
                                        Type type,
                                        VkImageLayout initialLayout) {
            m_Descriptions.EmplaceBack(initializers::AttachmentDescription(
                format,
                samplerCount,
                loadOp,
                storeOp,
                stencilLoadOp,
                stencilStoreOp,
                finalLayout,
                initialLayout));

            switch (type) {
            case Type::eColor:
                m_ColorReferences.EmplaceBack(initializers::AttachmentReference(
                    static_cast<std::uint32_t>(m_Descriptions.GetSize() - 1u),
                    static_cast<VkImageLayout>(attachment)));
                break;

            case Type::eDepth:
                m_DepthReferences.EmplaceBack(initializers::AttachmentReference(
                    static_cast<std::uint32_t>(m_Descriptions.GetSize() - 1u),
                    static_cast<VkImageLayout>(attachment)));
                break;

            case Type::eResolve:
                m_ResolveReferences.EmplaceBack(initializers::AttachmentReference(
                    static_cast<std::uint32_t>(m_Descriptions.GetSize() - 1u),
                    static_cast<VkImageLayout>(attachment)));
                break;

            case Type::eInput:
                break;
            }
        }

        void Attachment::AddInputReference(VkImageLayout attachment, std::uint32_t index) {
            m_InputReferences.EmplaceBack(initializers::AttachmentReference(
                index,
                static_cast<VkImageLayout>(attachment)));
        }

        Array<VkAttachmentDescription>& Attachment::GetDescriptions() noexcept {
            return m_Descriptions;
        }

        const Array<VkAttachmentDescription>& Attachment::GetDescriptions() const noexcept {
            return m_Descriptions;
        }

        Array<VkAttachmentReference>& Attachment::GetColorReferences() noexcept {
            return m_ColorReferences;
        }

        const Array<VkAttachmentReference>& Attachment::GetColorReferences() const noexcept {
            return m_ColorReferences;
        }

        Array<VkAttachmentReference>& Attachment::GetDepthReferences() noexcept {
            return m_DepthReferences;
        }

        const Array<VkAttachmentReference>& Attachment::GetDepthReferences() const noexcept {
            return m_DepthReferences;
        }

        Array<VkAttachmentReference>& Attachment::GetResolveReferences() noexcept {
            return m_ResolveReferences;
        }

        const Array<VkAttachmentReference>& Attachment::GetResolveReferences() const noexcept {
            return m_ResolveReferences;
        }

        Array<VkAttachmentReference>& Attachment::GetInputReferences() noexcept {
            return m_InputReferences;
        }

        const Array<VkAttachmentReference>& Attachment::GetInputReferences() const noexcept {
            return m_InputReferences;
        }
    } // namespace vk
} // namespace adh
