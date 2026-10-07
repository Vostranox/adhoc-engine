#include "ComputePipeline.hpp"
#include "Context.hpp"
#include "Initializers.hpp"

namespace adh {
    namespace vk {
        ComputePipeline::ComputePipeline() noexcept : m_Pipeline{ VK_NULL_HANDLE } {
        }

        ComputePipeline::ComputePipeline(VkPipelineLayout layout, VkPipelineShaderStageCreateInfo stage) ADH_NOEXCEPT {
            Create(layout, stage);
        }

        ComputePipeline::ComputePipeline(ComputePipeline&& rhs) noexcept {
            MoveConstruct(Move(rhs));
        }

        ComputePipeline& ComputePipeline::operator=(ComputePipeline&& rhs) noexcept {
            Clear();
            MoveConstruct(Move(rhs));
            return *this;
        }

        ComputePipeline::~ComputePipeline() {
            Clear();
        }

        void ComputePipeline::Create(VkPipelineLayout layout, VkPipelineShaderStageCreateInfo stage) ADH_NOEXCEPT {
            auto info{ initializers::ComputePipelineCreateInfo(layout, stage) };

            ADH_THROW(vkCreateComputePipelines(
                          Context::Get()->GetDevice(),
                          VK_NULL_HANDLE,
                          1u,
                          &info,
                          nullptr,
                          &m_Pipeline) == VK_SUCCESS,
                      "Failed to create compute pipeline!");
        }

        void ComputePipeline::Bind(VkCommandBuffer commandBuffer) noexcept {
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_Pipeline);
        }

        void ComputePipeline::Dispatch(VkCommandBuffer commandBuffer, std::uint32_t x, std::uint32_t y, std::uint32_t z) noexcept {
            vkCmdDispatch(commandBuffer, x, y, z);
        }

        std::uint32_t ComputePipeline::GroupCount(std::uint32_t count, std::uint32_t localSize) noexcept {
            return (count + localSize - 1u) / localSize;
        }

        void ComputePipeline::Destroy() noexcept {
            Clear();
        }

        ComputePipeline::operator VkPipeline() noexcept {
            return m_Pipeline;
        }

        ComputePipeline::operator VkPipeline() const noexcept {
            return m_Pipeline;
        }

        void ComputePipeline::MoveConstruct(ComputePipeline&& rhs) noexcept {
            m_Pipeline     = rhs.m_Pipeline;
            rhs.m_Pipeline = VK_NULL_HANDLE;
        }

        void ComputePipeline::Clear() noexcept {
            if (m_Pipeline != VK_NULL_HANDLE) {
                vkDestroyPipeline(Context::Get()->GetDevice(), m_Pipeline, nullptr);
                m_Pipeline = VK_NULL_HANDLE;
            }
        }
    } // namespace vk
} // namespace adh
