#include "FullscreenPass.hpp"
#include <Vulkan/Attachments.hpp>
#include <Vulkan/Memory.hpp>
#include <Vulkan/Shader.hpp>
#include <Vulkan/Subpass.hpp>
#include <Vulkan/VertexLayout.hpp>

namespace adh {
    void CreateColorPass(vk::RenderPass& renderPass, VkFormat format) {
        vk::Attachment attachment;
        attachment.AddDescription(
            format,
            VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_CLEAR,
            VK_ATTACHMENT_STORE_OP_STORE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            vk::Attachment::Type::eColor);

        vk::Subpass subpass;
        subpass.AddDescription(VK_PIPELINE_BIND_POINT_GRAPHICS, attachment);
        subpass.AddDependencies(
            VK_SUBPASS_EXTERNAL,
            0u,
            VkPipelineStageFlagBits(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT),
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_NONE_KHR,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

        subpass.AddDependencies(
            0u,
            VK_SUBPASS_EXTERNAL,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT);

        Array<VkClearValue> clearValues;
        clearValues.Resize(1);
        clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

        renderPass.Create(attachment, subpass, {}, Move(clearValues));
    }

    vk::Image CreateColorTarget(VkExtent2D extent, VkFormat format) {
        vk::Image image{
            { extent.width, extent.height, 1u },
            format,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_TYPE_2D,
            VkImageCreateFlagBits(0),
            1u,
            1u,
            VK_SAMPLE_COUNT_1_BIT,
            VkImageUsageFlagBits(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT),
            VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_VIEW_TYPE_2D,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            VK_SHARING_MODE_EXCLUSIVE
        };

        vk::TransferImageLayout(
            image,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_ASPECT_COLOR_BIT);

        return image;
    }

    void CreateFullscreenPipeline(vk::GraphicsPipeline& pipeline, const vk::PipelineLayout& layout, const vk::RenderPass& renderPass, const char* fragmentShader) {
        vk::Shader shader("hdr.vert", fragmentShader);

        vk::VertexLayout vertexLayout;
        vertexLayout.Create();

        pipeline.Create(shader, vertexLayout, layout, renderPass,
                        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE,
                        VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 0.0f, VK_TRUE);
    }

    void UpdateInputImage(vk::DescriptorSet& set, std::uint32_t binding, const VkDescriptorImageInfo& image) {
        set.Update(image, 0u, binding, 0u, 1u, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    }
} // namespace adh
