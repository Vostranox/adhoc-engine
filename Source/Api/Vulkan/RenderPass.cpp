#include "RenderPass.hpp"
#include "Context.hpp"
#include "Initializers.hpp"

namespace adh {
    namespace vk {
        RenderPass::RenderPass() noexcept : m_RenderPass{ VK_NULL_HANDLE },
                                            m_RenderArea{} {
        }

        RenderPass::RenderPass(const Attachment& attachment, const Subpass& subpass, VkRect2D renderArea, Array<VkClearValue> clearValues) ADH_NOEXCEPT {
            Create(attachment, subpass, renderArea, Move(clearValues));
        }

        RenderPass::RenderPass(RenderPass&& rhs) noexcept {
            MoveConstruct(Move(rhs));
        }

        RenderPass& RenderPass::operator=(RenderPass&& rhs) noexcept {
            Clear();
            MoveConstruct(Move(rhs));
            return *this;
        }

        RenderPass::~RenderPass() {
            Clear();
        }

        void RenderPass::Create(const Attachment& attachment, const Subpass& subpass, VkRect2D renderArea, Array<VkClearValue> clearValues) ADH_NOEXCEPT {
            auto info{
                initializers::RenderPassCreateInfo(
                    attachment.GetDescriptions(),
                    subpass.GetDescriptions(),
                    subpass.GetDependencies())
            };

            ADH_THROW(vkCreateRenderPass(Context::Get()->GetDevice(), &info, nullptr, &m_RenderPass) == VK_SUCCESS,
                      "Failed to create render pass!");

            m_RenderArea  = renderArea;
            m_ClearValues = Move(clearValues);
        }

        void RenderPass::Destroy() noexcept {
            Clear();
        }

        void RenderPass::Begin(VkCommandBuffer commandBuffer, VkFramebuffer framebuffer) noexcept {
            auto info{ initializers::RenderPassBeginInfo(m_RenderPass, framebuffer, m_RenderArea, static_cast<std::uint32_t>(m_ClearValues.GetSize()), m_ClearValues.GetData()) };

            vkCmdBeginRenderPass(
                commandBuffer,
                &info,
                VK_SUBPASS_CONTENTS_INLINE);
        }

        void RenderPass::End(VkCommandBuffer commandBuffer) noexcept {
            vkCmdEndRenderPass(commandBuffer);
        }

        void RenderPass::UpdateRenderArea(VkRect2D renderArea) noexcept {
            m_RenderArea = renderArea;
        }

        void RenderPass::UpdateClearValues(Array<VkClearValue> clearValues) noexcept {
            m_ClearValues = Move(clearValues);
        }

        void RenderPass::SetClearColor(float r, float g, float b, float a) noexcept {
            m_ClearValues[0].color = { { r, g, b, a } };
        }

        void RenderPass::SetClearDepthStencil(float depth, float stencil) noexcept {
            m_ClearValues[1].depthStencil = { depth, static_cast<std::uint32_t>(stencil) };
        }

        RenderPass::operator VkRenderPass() noexcept {
            return m_RenderPass;
        }

        RenderPass::operator VkRenderPass() const noexcept {
            return m_RenderPass;
        }

        void RenderPass::MoveConstruct(RenderPass&& rhs) noexcept {
            m_RenderPass     = rhs.m_RenderPass;
            rhs.m_RenderPass = VK_NULL_HANDLE;
        }

        void RenderPass::Clear() noexcept {
            if (m_RenderPass != VK_NULL_HANDLE) {
                vkDestroyRenderPass(Context::Get()->GetDevice(), m_RenderPass, nullptr);
                m_RenderPass = VK_NULL_HANDLE;
            }
        }
    } // namespace vk
} // namespace adh
