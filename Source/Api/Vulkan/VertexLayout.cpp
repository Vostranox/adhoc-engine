#include "VertexLayout.hpp"
#include "Initializers.hpp"

namespace adh {
    namespace vk {
        VertexLayout::VertexLayout() noexcept : m_PipelineVertexInputStateCreateInfo{} {
        }

        VertexLayout::VertexLayout(VertexLayout&& rhs) noexcept {
            MoveConstruct(Move(rhs));
        }

        VertexLayout& VertexLayout::operator=(VertexLayout&& rhs) noexcept {
            MoveConstruct(Move(rhs));
            return *this;
        }

        void VertexLayout::AddBinding(std::uint32_t binding, std::uint32_t stride, VkVertexInputRate inputRate) noexcept {
            m_BindingDescription.EmplaceBack(initializers::VertexInputBindingDescription(binding, stride, inputRate));
        }

        void VertexLayout::AddAttribute(std::uint32_t location, std::uint32_t binding, VkFormat format, std::uint32_t offset) noexcept {
            m_AtttributeDescription.EmplaceBack(initializers::VertexInputAttributeDescription(location, binding, format, offset));
        }

        void VertexLayout::Create() noexcept {
            m_PipelineVertexInputStateCreateInfo = initializers::PipelineVertexInputStateCreateInfo(m_BindingDescription, m_AtttributeDescription);
        }

        VkPipelineVertexInputStateCreateInfo& VertexLayout::Get() noexcept {
            return m_PipelineVertexInputStateCreateInfo;
        }

        const VkPipelineVertexInputStateCreateInfo& VertexLayout::Get() const noexcept {
            return m_PipelineVertexInputStateCreateInfo;
        }

        void VertexLayout::MoveConstruct(VertexLayout&& rhs) noexcept {
            m_BindingDescription                 = Move(rhs.m_BindingDescription);
            m_AtttributeDescription              = Move(rhs.m_AtttributeDescription);
            m_PipelineVertexInputStateCreateInfo = rhs.m_PipelineVertexInputStateCreateInfo;
        }
    } // namespace vk
} // namespace adh
