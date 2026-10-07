#include "Bloom.hpp"
#include "FullscreenPass.hpp"

#include <Utility.hpp>
#include <Vulkan/Scissor.hpp>
#include <Vulkan/Viewport.hpp>

namespace adh {
    constexpr std::size_t maxLevels{ 8u };

    void Bloom::Create(const vk::Sampler& sampler, std::uint32_t imageCount) {
        m_Sampler    = &sampler;
        m_ImageCount = imageCount;
        CreateColorPass(m_RenderPass, hdrFormat);

        m_DownsampleLayout.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_DownsampleLayout.CreateSet();
        m_DownsampleLayout.AddPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(int), 0);
        m_DownsampleLayout.Create();
        CreateFullscreenPipeline(m_DownsamplePipeline, m_DownsampleLayout, m_RenderPass, "bloom_downsample.frag");

        m_UpsampleLayout.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_UpsampleLayout.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_UpsampleLayout.CreateSet();
        m_UpsampleLayout.AddPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(float), 0);
        m_UpsampleLayout.Create();
        CreateFullscreenPipeline(m_UpsamplePipeline, m_UpsampleLayout, m_RenderPass, "bloom_upsample.frag");
    }

    void Bloom::CreateTargets(BloomTargets& targets, VkExtent2D viewExtent, VkDescriptorImageInfo hdrInput) {
        Array<VkExtent2D> levels;
        VkExtent2D extent{ viewExtent.width / 2, viewExtent.height / 2 };
        while (levels.GetSize() != maxLevels && extent.width != 0 && extent.height != 0) {
            levels.EmplaceBack(extent);
            extent.width /= 2;
            extent.height /= 2;
        }
        ADH_THROW(levels.GetSize() >= 2, "The view is too small for bloom!");

        targets.downsample.Resize(levels.GetSize());
        for (std::size_t i{}; i != targets.downsample.GetSize(); ++i) {
            CreatePass(targets.downsample[i], levels[i], m_DownsampleLayout);
            UpdateInputImage(targets.downsample[i].inputs, 0u, i == 0 ? hdrInput : targets.downsample[i - 1].output);
        }

        targets.upsample.Resize(levels.GetSize() - 1);
        for (std::size_t i{ targets.upsample.GetSize() }; i-- != 0;) {
            const BloomTargets::Pass& smaller{ i + 1 == targets.upsample.GetSize() ? targets.downsample[targets.downsample.GetSize() - 1] : targets.upsample[i + 1] };

            CreatePass(targets.upsample[i], levels[i], m_UpsampleLayout);
            UpdateInputImage(targets.upsample[i].inputs, 0u, targets.downsample[i].output);
            UpdateInputImage(targets.upsample[i].inputs, 1u, smaller.output);
        }
    }

    void Bloom::Draw(VkCommandBuffer cmd, std::uint32_t imageIndex, BloomTargets& targets, const RenderSettings& settings) {
        for (std::size_t i{}; i != targets.downsample.GetSize(); ++i) {
            const int karisAverage{ i == 0 };

            BeginPass(cmd, imageIndex, targets.downsample[i], m_DownsamplePipeline);
            vkCmdPushConstants(cmd, m_DownsampleLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0u, sizeof(karisAverage), &karisAverage);
            vkCmdDraw(cmd, 3, 1, 0, 0);
            m_RenderPass.End(cmd);
        }

        for (std::size_t i{ targets.upsample.GetSize() }; i-- != 0;) {
            BeginPass(cmd, imageIndex, targets.upsample[i], m_UpsamplePipeline);
            vkCmdPushConstants(cmd, m_UpsampleLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0u, sizeof(settings.bloomRadius), &settings.bloomRadius);
            vkCmdDraw(cmd, 3, 1, 0, 0);
            m_RenderPass.End(cmd);
        }
    }

    void Bloom::CreatePass(BloomTargets::Pass& pass, VkExtent2D extent, const vk::PipelineLayout& layout) {
        pass.extent = extent;
        pass.image  = CreateColorTarget(extent, hdrFormat);

        VkImageView attachments[]{ pass.image.GetImageView() };
        pass.framebuffer.Create(m_RenderPass, std::size(attachments), attachments, extent, 1u);

        pass.output.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        pass.output.sampler     = *m_Sampler;
        pass.output.imageView   = pass.image.GetImageView();

        pass.inputs.Initialize(VK_PIPELINE_BIND_POINT_GRAPHICS, layout, m_ImageCount);
        pass.inputs.AddPool(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2);
        pass.inputs.Create(layout.GetSetLayout());
    }

    void Bloom::BeginPass(VkCommandBuffer cmd, std::uint32_t imageIndex, BloomTargets::Pass& pass, vk::GraphicsPipeline& pipeline) {
        vk::Viewport viewport{ pass.extent, false };
        viewport.Set(cmd);
        vk::Scissor scissor{ pass.extent };
        scissor.Set(cmd);

        m_RenderPass.UpdateRenderArea({ {}, pass.extent });
        m_RenderPass.Begin(cmd, pass.framebuffer);
        pipeline.Bind(cmd);
        pass.inputs.Bind(cmd, imageIndex);
    }
} // namespace adh
