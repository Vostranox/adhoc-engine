#pragma once
#include "Attachments.hpp"
#include "Subpass.hpp"
#include <Std/Array.hpp>
#include <Utility.hpp>
#include <vulkan/vulkan.h>

namespace adh {
    namespace vk {
        class RenderPass {
          public:
            RenderPass() noexcept;

            RenderPass(const Attachment& attachment, const Subpass& subpass, VkRect2D renderArea, Array<VkClearValue> clearValues) ADH_NOEXCEPT;

            RenderPass(const RenderPass& rhs) = delete;

            RenderPass& operator=(const RenderPass& rhs) = delete;

            RenderPass(RenderPass&& rhs) noexcept;

            RenderPass& operator=(RenderPass&& rhs) noexcept;

            ~RenderPass();

            void Create(const Attachment& attachment, const Subpass& subpass, VkRect2D renderArea, Array<VkClearValue> clearValues) ADH_NOEXCEPT;

            void Destroy() noexcept;

            void Begin(VkCommandBuffer commandBuffer, VkFramebuffer framebuffer) noexcept;

            void End(VkCommandBuffer commandBuffer) noexcept;

            void UpdateRenderArea(VkRect2D renderArea) noexcept;

            void UpdateClearValues(Array<VkClearValue> clearValues) noexcept;

            void SetClearColor(float r, float g, float b, float a) noexcept;

            void SetClearDepthStencil(float depth, float stencil) noexcept;

            operator VkRenderPass() noexcept;

            operator VkRenderPass() const noexcept;

          private:
            void MoveConstruct(RenderPass&& rhs) noexcept;

            void Clear() noexcept;

          private:
            VkRenderPass m_RenderPass;
            VkRect2D m_RenderArea;
            Array<VkClearValue> m_ClearValues;
        };
    } // namespace vk
} // namespace adh
