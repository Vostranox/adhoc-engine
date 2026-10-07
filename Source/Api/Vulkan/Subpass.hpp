#pragma once
#include "Attachments.hpp"
#include <Std/Array.hpp>
#include <cstdint>
#include <vulkan/vulkan.h>

namespace adh {
    namespace vk {
        class Subpass {
          public:
            void AddDescription(VkPipelineBindPoint bindPoint, const Attachment& attachment);

            void AddDependencies(
                std::uint32_t srcSubpass,
                std::uint32_t dstSubpass,
                VkPipelineStageFlagBits srcStageMask,
                VkPipelineStageFlagBits dstStageMask,
                VkAccessFlagBits srcAccessMask,
                VkAccessFlagBits dstAccessMask,
                VkDependencyFlags dependencyFlags = 0u);

            Array<VkSubpassDescription>& GetDescriptions() noexcept;

            const Array<VkSubpassDescription>& GetDescriptions() const noexcept;

            Array<VkSubpassDependency>& GetDependencies() noexcept;

            const Array<VkSubpassDependency>& GetDependencies() const noexcept;

          private:
            Array<VkSubpassDescription> m_Descriptions;
            Array<VkSubpassDependency> m_Dependencies;
        };
    } // namespace vk
} // namespace adh
