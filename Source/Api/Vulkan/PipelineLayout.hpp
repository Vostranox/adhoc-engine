#pragma once
#include <Std/Array.hpp>
#include <Utility.hpp>
#include <vulkan/vulkan.h>

namespace adh {
    namespace vk {
        class PipelineLayout {
          public:
            PipelineLayout() noexcept;

            PipelineLayout(const PipelineLayout&) = delete;

            PipelineLayout& operator=(const PipelineLayout&) = delete;

            PipelineLayout(PipelineLayout&& rhs) noexcept;

            PipelineLayout& operator=(PipelineLayout&& rhs) noexcept;

            ~PipelineLayout();

            void Create() ADH_NOEXCEPT;

            void Destroy() noexcept;

            void AddBinding(std::uint32_t binding, VkDescriptorType type, std::uint32_t count, VkShaderStageFlags shaderStage, const VkSampler* immutableSampler = nullptr);

            void AddPushConstant(VkShaderStageFlags shaderStage, std::uint32_t size, std::uint32_t offset);

            void CreateSet() ADH_NOEXCEPT;

            VkPipelineLayout GetPipelineLayout() noexcept;

            VkPipelineLayout GetPipelineLayout() const noexcept;

            Array<VkDescriptorSetLayout>& GetSetLayout() noexcept;

            const Array<VkDescriptorSetLayout>& GetSetLayout() const noexcept;

            VkPushConstantRange* GetPushConstant() noexcept;

            const VkPushConstantRange* GetPushConstant() const noexcept;

            operator VkPipelineLayout();

            operator VkPipelineLayout() const;

          private:
            void MoveConstruct(PipelineLayout&& rhs) noexcept;

            void Clear() noexcept;

          private:
            VkPipelineLayout m_PipelineLayout;
            Array<VkDescriptorSetLayoutBinding> m_LayoutBindings;
            Array<VkDescriptorSetLayout> m_DescriptorSetLayout;
            Array<VkPushConstantRange> m_PushConstants;
        };
    } // namespace vk
} // namespace adh
