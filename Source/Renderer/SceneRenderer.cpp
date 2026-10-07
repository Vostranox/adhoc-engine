#include "SceneRenderer.hpp"
#include "FullscreenPass.hpp"

#include <Vertex.hpp>
#include <Vulkan/Attachments.hpp>
#include <Vulkan/Context.hpp>
#include <Vulkan/Scissor.hpp>
#include <Vulkan/Shader.hpp>
#include <Vulkan/Subpass.hpp>
#include <Vulkan/Tools.hpp>
#include <Vulkan/VertexLayout.hpp>
#include <Vulkan/Viewport.hpp>

#include <algorithm>

namespace adh {
    constexpr VkFormat ldrFormat{ VK_FORMAT_B8G8R8A8_UNORM };
    constexpr VkFormat entityIdFormat{ VK_FORMAT_R32G32_UINT };
    constexpr VkFormat gAlbedoMetallicFormat{ VK_FORMAT_R8G8B8A8_SRGB };
    constexpr VkFormat gNormalRoughnessFormat{ VK_FORMAT_R16G16B16A16_SFLOAT };
    constexpr VkFormat gDepthFormat{ VK_FORMAT_R32_SFLOAT };
    constexpr VkFormat gVertexNormalFormat{ VK_FORMAT_A2B10G10R10_UNORM_PACK32 };

    constexpr float outlineColor[4]{ 1.0f, 0.5f, 0.0f, 1.0f };
    constexpr float outlineScale{ 1.05f };

    struct Tonemap {
        float bloomStrength;
        float exposure;
    };

    constexpr std::uint32_t materialOffset{ sizeof(xmm::Matrix) };
    constexpr std::uint32_t entityIdOffset{ materialOffset + sizeof(Material) };
    constexpr std::uint32_t tilesPerRowOffset{ entityIdOffset + sizeof(std::uint32_t[2]) };
    static_assert(entityIdOffset == 112u && tilesPerRowOffset == 120u, "pbr.frag reads the entity id at offset 112, the tiles per row at 120");

    constexpr std::uint32_t tileSize{ 16u };
    constexpr std::uint32_t tileStride{ 256u };

    constexpr vk::DepthStencilState marksStencil{
        .stencilTest = VK_TRUE,
        .stencil     = { .failOp      = VK_STENCIL_OP_KEEP,
                         .passOp      = VK_STENCIL_OP_REPLACE,
                         .depthFailOp = VK_STENCIL_OP_REPLACE,
                         .compareOp   = VK_COMPARE_OP_ALWAYS,
                         .compareMask = 0xFFu,
                         .writeMask   = 0xFFu,
                         .reference   = 1u }
    };

    static void CreateMeshVertexLayout(vk::VertexLayout& vertexLayout, bool onlyPosition) {
        vertexLayout.AddBinding(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);
        vertexLayout.AddAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, ADH_OFFSET(Vertex, position));
        if (!onlyPosition) {
            vertexLayout.AddAttribute(1, 0, VK_FORMAT_R32G32B32_SFLOAT, ADH_OFFSET(Vertex, normals));
            vertexLayout.AddAttribute(2, 0, VK_FORMAT_R32G32_SFLOAT, ADH_OFFSET(Vertex, textureCoords));
            vertexLayout.AddAttribute(3, 0, VK_FORMAT_R32G32B32A32_SFLOAT, ADH_OFFSET(Vertex, tangent));
        }
        vertexLayout.Create();
    }

    static void AddColorAttachment(vk::Attachment& attachment, VkFormat format, VkImageLayout finalLayout) {
        attachment.AddDescription(
            format,
            VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_CLEAR,
            VK_ATTACHMENT_STORE_OP_STORE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            finalLayout,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            vk::Attachment::Type::eColor);
    }

    static void SetViewportAndScissor(VkCommandBuffer cmd, VkExtent2D extent) {
        vk::Viewport viewport{ extent, false };
        viewport.Set(cmd);
        vk::Scissor scissor{ extent };
        scissor.Set(cmd);
    }

    static void BufferBarrier(VkCommandBuffer cmd, VkBuffer buffer, VkPipelineStageFlags srcStage, VkAccessFlags srcAccess,
                              VkPipelineStageFlags dstStage, VkAccessFlags dstAccess) {
        const auto barrier{ vk::initializers::BufferMemoryBarrier(srcAccess, dstAccess, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
                                                                  buffer, 0u, VK_WHOLE_SIZE) };
        vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0u, 0u, nullptr, 1u, &barrier, 0u, nullptr);
    }

    void SceneRenderer::Create(const vk::RenderPass& presentPass, const vk::Sampler& sampler, std::uint32_t imageCount) {
        m_Sampler     = &sampler;
        m_ImageCount  = imageCount;
        m_DepthFormat = vk::tools::GetSupportedDepthStencilFormat(vk::Context::Get()->GetPhysicalDevice());
        m_NearestSampler.Create(VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_COMPARE_OP_NEVER);

        ADH_THROW(vk::tools::GetPhysicalDeviceFeatures(vk::Context::Get()->GetPhysicalDevice()).independentBlend,
                  "The scene pass needs independentBlend!");

        CreateSceneLayout();
        CreateLitPass(m_ScenePass, false, false);
        {
            vk::Shader shader("pbr.vert", "pbr.frag");
            vk::VertexLayout vertexLayout;
            CreateMeshVertexLayout(vertexLayout, false);

            const VkPipelineColorBlendAttachmentState blendStates[]{
                vk::initializers::PipelineColorBlendAttachmentState(VK_TRUE),
                vk::initializers::PipelineColorBlendAttachmentState(VK_FALSE),
            };
            m_PbrPipeline.Create(shader, vertexLayout, m_SceneLayout, m_ScenePass,
                                 VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE,
                                 VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 0.0f, blendStates);
            m_OutlinedPbrPipeline.Create(shader, vertexLayout, m_SceneLayout, m_ScenePass,
                                         VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE,
                                         VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 0.0f, blendStates, marksStencil);
        }

        lights.Create(imageCount);
        shadows.Create();

        skybox.Create(m_ScenePass);
        particles.Create(m_ScenePass, imageCount);

        CreateForwardPlus();
        CreateDeferred();

        m_Bloom.Create(sampler, imageCount);

        CreateColorPass(m_LdrPass, ldrFormat);
        m_TonemapLayout.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_TonemapLayout.AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_TonemapLayout.CreateSet();
        m_TonemapLayout.AddPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(Tonemap), 0);
        m_TonemapLayout.Create();
        CreateFullscreenPipeline(m_TonemapPipeline, m_TonemapLayout, m_LdrPass, "hdr.frag");

        m_PresentLayout.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_PresentLayout.CreateSet();
        m_PresentLayout.Create();
        CreateFullscreenPipeline(m_PresentPipeline, m_PresentLayout, presentPass, "present.frag");

        CreateOutlinePass();
        {
            m_OutlineLayout.AddPushConstant(VK_SHADER_STAGE_VERTEX_BIT, sizeof(xmm::Matrix), 0);
            m_OutlineLayout.AddPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(outlineColor), sizeof(xmm::Matrix));
            m_OutlineLayout.Create();

            vk::Shader shader("outline.vert", "outline.frag");
            vk::VertexLayout vertexLayout;
            CreateMeshVertexLayout(vertexLayout, true);

            const auto blendState{ vk::initializers::PipelineColorBlendAttachmentState(VK_FALSE) };
            const vk::DepthStencilState outsideStencil{
                .depthTest   = VK_FALSE,
                .depthWrite  = VK_FALSE,
                .stencilTest = VK_TRUE,
                .stencil     = { .failOp      = VK_STENCIL_OP_KEEP,
                                 .passOp      = VK_STENCIL_OP_KEEP,
                                 .depthFailOp = VK_STENCIL_OP_KEEP,
                                 .compareOp   = VK_COMPARE_OP_NOT_EQUAL,
                                 .compareMask = 0xFFu,
                                 .writeMask   = 0u,
                                 .reference   = 1u }
            };
            m_OutlinePipeline.Create(shader, vertexLayout, m_OutlineLayout, m_OutlinePass,
                                     VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE,
                                     VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 0.0f, { &blendState, 1u }, outsideStencil);
        }
    }

    const vk::PipelineLayout& SceneRenderer::GetSceneLayout() const noexcept {
        return m_SceneLayout;
    }

    void SceneRenderer::Resize(SceneView& view, VkExtent2D extent) {
        view        = SceneView{};
        view.extent = extent;

        view.hdr = CreateColorTarget(extent, hdrFormat);
        view.depth.Create(
            { extent.width, extent.height, 1u },
            m_DepthFormat,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_TYPE_2D,
            VkImageCreateFlagBits(0),
            1u,
            1u,
            VK_SAMPLE_COUNT_1_BIT,
            VkImageUsageFlagBits(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
            VkImageAspectFlagBits(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT),
            VK_IMAGE_VIEW_TYPE_2D,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            VK_SHARING_MODE_EXCLUSIVE);
        view.depthOnly.Create(view.depth.GetImage(), VK_IMAGE_VIEW_TYPE_2D, m_DepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1u, 1u, 0u);
        view.entityIds.Create(
            { extent.width, extent.height, 1u },
            entityIdFormat,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_TYPE_2D,
            VkImageCreateFlagBits(0),
            1u,
            1u,
            VK_SAMPLE_COUNT_1_BIT,
            VkImageUsageFlagBits(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT),
            VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_VIEW_TYPE_2D,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            VK_SHARING_MODE_EXCLUSIVE);
        VkImageView sceneAttachments[]{ view.hdr.GetImageView(), view.depth.GetImageView(), view.entityIds.GetImageView() };
        view.sceneFramebuffer.Create(m_ScenePass, std::size(sceneAttachments), sceneAttachments, extent, 1u);
        VkImageView depthAttachments[]{ view.depth.GetImageView() };
        view.depthFramebuffer.Create(m_DepthPass, std::size(depthAttachments), depthAttachments, extent, 1u);

        const VkDeviceSize tileCount{ vk::ComputePipeline::GroupCount(extent.width, tileSize) * vk::ComputePipeline::GroupCount(extent.height, tileSize) };
        view.lightTiles.Create(tileCount * tileStride * sizeof(std::uint32_t), 1u, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        view.gAlbedoMetallic  = CreateColorTarget(extent, gAlbedoMetallicFormat);
        view.gNormalRoughness = CreateColorTarget(extent, gNormalRoughnessFormat);
        view.gDepth           = CreateColorTarget(extent, gDepthFormat);
        view.gVertexNormal    = CreateColorTarget(extent, gVertexNormalFormat);
        VkImageView gBufferAttachments[]{ view.hdr.GetImageView(), view.depth.GetImageView(), view.entityIds.GetImageView(),
                                          view.gAlbedoMetallic.GetImageView(), view.gNormalRoughness.GetImageView(), view.gDepth.GetImageView(),
                                          view.gVertexNormal.GetImageView() };
        view.gBufferFramebuffer.Create(m_GBufferPass, std::size(gBufferAttachments), gBufferAttachments, extent, 1u);

        view.inputs.Initialize(VK_PIPELINE_BIND_POINT_GRAPHICS, m_SceneLayout, 1u);
        view.inputs.AddPool(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
        view.inputs.AddPool(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5);
        view.inputs.Create({ m_SceneLayout.GetSetLayout()[3] });
        view.inputs.Update(view.lightTiles, 0u, VK_WHOLE_SIZE, 0u, 0u, 0u, 1u, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        UpdateInputImage(view.inputs, 1u, { m_NearestSampler, view.depthOnly, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL });
        UpdateInputImage(view.inputs, 2u, { m_NearestSampler, view.gAlbedoMetallic.GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
        UpdateInputImage(view.inputs, 3u, { m_NearestSampler, view.gNormalRoughness.GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
        UpdateInputImage(view.inputs, 4u, { m_NearestSampler, view.gDepth.GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
        UpdateInputImage(view.inputs, 5u, { m_NearestSampler, view.gVertexNormal.GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });

        const VkDescriptorImageInfo hdr{ *m_Sampler, view.hdr.GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
        m_Bloom.CreateTargets(view.bloom, extent, hdr);

        view.ldr = CreateColorTarget(extent, ldrFormat);
        VkImageView ldrAttachments[]{ view.ldr.GetImageView() };
        view.ldrFramebuffer.Create(m_LdrPass, std::size(ldrAttachments), ldrAttachments, extent, 1u);
        VkImageView outlineAttachments[]{ view.ldr.GetImageView(), view.depth.GetImageView() };
        view.outlineFramebuffer.Create(m_OutlinePass, std::size(outlineAttachments), outlineAttachments, extent, 1u);

        view.tonemapInputs.Initialize(VK_PIPELINE_BIND_POINT_GRAPHICS, m_TonemapLayout, m_ImageCount);
        view.tonemapInputs.AddPool(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2);
        view.tonemapInputs.Create(m_TonemapLayout.GetSetLayout());
        UpdateInputImage(view.tonemapInputs, 1u, hdr);
        UpdateInputImage(view.tonemapInputs, 2u, view.bloom.upsample[0].output);

        view.presentInput.Initialize(VK_PIPELINE_BIND_POINT_GRAPHICS, m_PresentLayout, m_ImageCount);
        view.presentInput.AddPool(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
        view.presentInput.Create(m_PresentLayout.GetSetLayout());
        UpdateInputImage(view.presentInput, 0u, { *m_Sampler, view.ldr.GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
    }

    void SceneRenderer::Render(VkCommandBuffer cmd, std::uint32_t imageIndex, SceneView& view, const SceneCamera& camera, const SunCascades& cascades,
                               vk::DescriptorSet& cameraSet, std::span<const MeshDraw> meshes, ecs::Entity outlined) {
        shadows.Render(cmd, cascades, meshes);

        switch (settings.renderPath) {
        case RenderPath::eForward:
            RenderForward(cmd, imageIndex, view, camera, cameraSet, meshes, outlined);
            break;
        case RenderPath::eForwardPlus:
            RenderForwardPlus(cmd, imageIndex, view, camera, cameraSet, meshes, outlined);
            break;
        case RenderPath::eDeferred:
            RenderDeferred(cmd, imageIndex, view, camera, cameraSet, meshes, outlined);
            break;
        }

        m_Bloom.Draw(cmd, imageIndex, view.bloom, settings);

        m_LdrPass.UpdateRenderArea({ {}, view.extent });
        m_LdrPass.Begin(cmd, view.ldrFramebuffer);
        SetViewportAndScissor(cmd, view.extent);
        m_TonemapPipeline.Bind(cmd);
        view.tonemapInputs.Bind(cmd, imageIndex);
        const Tonemap tonemap{ settings.bloomStrength, settings.exposure };
        vkCmdPushConstants(cmd, m_TonemapLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0u, sizeof(tonemap), &tonemap);
        vkCmdDraw(cmd, 3, 1, 0, 0);
        m_LdrPass.End(cmd);

        for (const MeshDraw& draw : meshes) {
            if (draw.entity == outlined) {
                DrawOutline(cmd, view, *draw.mesh, camera.viewProjection * draw.model);
            }
        }
    }

    void SceneRenderer::Present(VkCommandBuffer cmd, std::uint32_t imageIndex, SceneView& view) {
        m_PresentPipeline.Bind(cmd);
        view.presentInput.Bind(cmd, imageIndex);
        vkCmdDraw(cmd, 3, 1, 0, 0);
    }

    void SceneRenderer::CreateSceneLayout() {
        m_SceneLayout.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT);
        m_SceneLayout.AddBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_SceneLayout.CreateSet();

        m_SceneLayout.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_SceneLayout.AddBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_SceneLayout.AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_SceneLayout.AddBinding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT);
        m_SceneLayout.CreateSet();

        for (std::uint32_t map{}; map != vk::TextureDescriptors::mapCount; ++map) {
            m_SceneLayout.AddBinding(map, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        }
        m_SceneLayout.CreateSet();

        m_SceneLayout.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT);
        m_SceneLayout.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
        for (std::uint32_t binding{ 2u }; binding != 6u; ++binding) {
            m_SceneLayout.AddBinding(binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        }
        m_SceneLayout.CreateSet();

        m_SceneLayout.AddPushConstant(VK_SHADER_STAGE_VERTEX_BIT, sizeof(xmm::Matrix), 0);
        m_SceneLayout.AddPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, tilesPerRowOffset + sizeof(std::uint32_t) - materialOffset, materialOffset);
        m_SceneLayout.Create();
    }

    void SceneRenderer::CreateLitPass(vk::RenderPass& pass, bool depthDrawn, bool colorDrawn) {
        vk::Attachment attachment;
        attachment.AddDescription(
            hdrFormat,
            VK_SAMPLE_COUNT_1_BIT,
            colorDrawn ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR,
            VK_ATTACHMENT_STORE_OP_STORE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            vk::Attachment::Type::eColor,
            colorDrawn ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED);

        attachment.AddDescription(
            m_DepthFormat,
            VK_SAMPLE_COUNT_1_BIT,
            depthDrawn ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            depthDrawn ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR,
            VK_ATTACHMENT_STORE_OP_STORE,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            vk::Attachment::Type::eDepth,
            depthDrawn ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED);

        attachment.AddDescription(
            entityIdFormat,
            VK_SAMPLE_COUNT_1_BIT,
            colorDrawn ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR,
            VK_ATTACHMENT_STORE_OP_STORE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            vk::Attachment::Type::eColor,
            colorDrawn ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED);

        vk::Subpass subpass;
        subpass.AddDescription(VK_PIPELINE_BIND_POINT_GRAPHICS, attachment);
        subpass.AddDependencies(
            VK_SUBPASS_EXTERNAL,
            0u,
            VkPipelineStageFlagBits(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT),
            VkPipelineStageFlagBits(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT),
            VkAccessFlagBits(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT),
            VkAccessFlagBits(VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT));

        subpass.AddDependencies(
            0u,
            VK_SUBPASS_EXTERNAL,
            VkPipelineStageFlagBits(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT),
            VkPipelineStageFlagBits(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT |
                                    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT),
            VkAccessFlagBits(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT),
            VkAccessFlagBits(VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT));

        Array<VkClearValue> clearValues;
        clearValues.Resize(3);
        clearValues[0].color        = { { 0.0f, 0.0f, 0.0f, 1.0f } };
        clearValues[1].depthStencil = { 1.0f, 0u };
        clearValues[2].color        = { .uint32 = { 0u, 0u, 0u, 0u } };

        pass.Create(attachment, subpass, {}, Move(clearValues));
    }

    void SceneRenderer::CreateDepthPass() {
        vk::Attachment attachment;
        attachment.AddDescription(
            m_DepthFormat,
            VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_CLEAR,
            VK_ATTACHMENT_STORE_OP_STORE,
            VK_ATTACHMENT_LOAD_OP_CLEAR,
            VK_ATTACHMENT_STORE_OP_STORE,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            vk::Attachment::Type::eDepth);

        vk::Subpass subpass;
        subpass.AddDescription(VK_PIPELINE_BIND_POINT_GRAPHICS, attachment);
        subpass.AddDependencies(
            VK_SUBPASS_EXTERNAL,
            0u,
            VkPipelineStageFlagBits(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                                    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT),
            VkPipelineStageFlagBits(VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT),
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VkAccessFlagBits(VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT));

        subpass.AddDependencies(
            0u,
            VK_SUBPASS_EXTERNAL,
            VkPipelineStageFlagBits(VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT),
            VkPipelineStageFlagBits(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                                    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT),
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VkAccessFlagBits(VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT));

        Array<VkClearValue> clearValues;
        clearValues.Resize(1);
        clearValues[0].depthStencil = { 1.0f, 0u };

        m_DepthPass.Create(attachment, subpass, {}, Move(clearValues));
    }

    void SceneRenderer::CreateForwardPlus() {
        CreateDepthPass();
        CreateLitPass(m_ForwardPlusPass, true, false);
        {
            vk::Shader shader("depth.vert");
            vk::VertexLayout vertexLayout;
            CreateMeshVertexLayout(vertexLayout, true);
            m_DepthPipeline.Create(shader, vertexLayout, m_SceneLayout, m_DepthPass,
                                   VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE,
                                   VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 0.0f, {});
            m_OutlinedDepthPipeline.Create(shader, vertexLayout, m_SceneLayout, m_DepthPass,
                                           VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE,
                                           VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 0.0f, {}, marksStencil);
        }
        {
            vk::Shader shader("lightcull.comp");
            m_CullPipeline.Create(m_SceneLayout, shader.Get());
        }
        {
            vk::Shader shader("pbr.vert", "pbr_tiled.frag");
            vk::VertexLayout vertexLayout;
            CreateMeshVertexLayout(vertexLayout, false);

            const VkPipelineColorBlendAttachmentState blendStates[]{
                vk::initializers::PipelineColorBlendAttachmentState(VK_TRUE),
                vk::initializers::PipelineColorBlendAttachmentState(VK_FALSE),
            };
            m_TiledPbrPipeline.Create(shader, vertexLayout, m_SceneLayout, m_ForwardPlusPass,
                                      VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE,
                                      VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 0.0f, blendStates, { .depthWrite = VK_FALSE });
        }
    }

    void SceneRenderer::CreateGBufferPass() {
        vk::Attachment attachment;
        AddColorAttachment(attachment, hdrFormat, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        attachment.AddDescription(
            m_DepthFormat,
            VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_CLEAR,
            VK_ATTACHMENT_STORE_OP_STORE,
            VK_ATTACHMENT_LOAD_OP_CLEAR,
            VK_ATTACHMENT_STORE_OP_STORE,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            vk::Attachment::Type::eDepth);
        AddColorAttachment(attachment, entityIdFormat, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        AddColorAttachment(attachment, gAlbedoMetallicFormat, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        AddColorAttachment(attachment, gNormalRoughnessFormat, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        AddColorAttachment(attachment, gDepthFormat, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        AddColorAttachment(attachment, gVertexNormalFormat, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vk::Subpass subpass;
        subpass.AddDescription(VK_PIPELINE_BIND_POINT_GRAPHICS, attachment);
        subpass.AddDependencies(
            VK_SUBPASS_EXTERNAL,
            0u,
            VkPipelineStageFlagBits(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT),
            VkPipelineStageFlagBits(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT),
            VkAccessFlagBits(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT),
            VkAccessFlagBits(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT));

        subpass.AddDependencies(
            0u,
            VK_SUBPASS_EXTERNAL,
            VkPipelineStageFlagBits(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT),
            VkPipelineStageFlagBits(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT),
            VkAccessFlagBits(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT),
            VkAccessFlagBits(VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT));

        Array<VkClearValue> clearValues;
        clearValues.Resize(7);
        clearValues[0].color        = { { 0.0f, 0.0f, 0.0f, 1.0f } };
        clearValues[1].depthStencil = { 1.0f, 0u };
        clearValues[2].color        = { .uint32 = { 0u, 0u, 0u, 0u } };
        clearValues[3].color        = { { 0.0f, 0.0f, 0.0f, 0.0f } };
        clearValues[4].color        = { { 0.0f, 0.0f, 0.0f, 0.0f } };
        clearValues[5].color        = { { 1.0f, 0.0f, 0.0f, 0.0f } };
        clearValues[6].color        = { { 0.0f, 0.0f, 0.0f, 0.0f } };

        m_GBufferPass.Create(attachment, subpass, {}, Move(clearValues));
    }

    void SceneRenderer::CreateDeferred() {
        CreateGBufferPass();
        CreateLitPass(m_DeferredPass, true, true);
        {
            vk::Shader shader("pbr.vert", "gbuffer.frag");
            vk::VertexLayout vertexLayout;
            CreateMeshVertexLayout(vertexLayout, false);

            const auto noBlending{ vk::initializers::PipelineColorBlendAttachmentState(VK_FALSE) };
            const VkPipelineColorBlendAttachmentState blendStates[]{ noBlending, noBlending, noBlending, noBlending, noBlending, noBlending };
            m_GBufferPipeline.Create(shader, vertexLayout, m_SceneLayout, m_GBufferPass,
                                     VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE,
                                     VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 0.0f, blendStates);
            m_OutlinedGBufferPipeline.Create(shader, vertexLayout, m_SceneLayout, m_GBufferPass,
                                             VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE,
                                             VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 0.0f, blendStates, marksStencil);
        }
        {
            vk::Shader shader("hdr.vert", "deferred_sun.frag");
            vk::VertexLayout vertexLayout;
            vertexLayout.Create();

            VkPipelineColorBlendAttachmentState additive{};
            additive.blendEnable         = VK_TRUE;
            additive.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            additive.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
            additive.colorBlendOp        = VK_BLEND_OP_ADD;
            additive.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            additive.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            additive.alphaBlendOp        = VK_BLEND_OP_ADD;
            additive.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            auto noEntityIds{ vk::initializers::PipelineColorBlendAttachmentState(VK_FALSE) };
            noEntityIds.colorWriteMask = 0u;
            const VkPipelineColorBlendAttachmentState blendStates[]{ additive, noEntityIds };
            m_SunPipeline.Create(shader, vertexLayout, m_SceneLayout, m_DeferredPass,
                                 VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE,
                                 VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 0.0f, blendStates, { .depthTest = VK_FALSE, .depthWrite = VK_FALSE });

            vk::Shader volumeShader("light_volume.vert", "light_volume.frag");
            vk::VertexLayout volumeLayout;
            CreateMeshVertexLayout(volumeLayout, true);
            m_LightVolumePipeline.Create(volumeShader, volumeLayout, m_SceneLayout, m_DeferredPass,
                                         VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_FRONT_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE,
                                         VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 0.0f, blendStates,
                                         { .depthWrite = VK_FALSE, .depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL });
        }
        m_LightVolume.Load(vk::Context::Get()->GetDataDirectory() + "Assets/Models/sphere.obj");
    }

    void SceneRenderer::CreateOutlinePass() {
        vk::Attachment attachment;
        attachment.AddDescription(
            ldrFormat,
            VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_LOAD,
            VK_ATTACHMENT_STORE_OP_STORE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            vk::Attachment::Type::eColor,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        attachment.AddDescription(
            m_DepthFormat,
            VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_LOAD,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_ATTACHMENT_LOAD_OP_LOAD,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            vk::Attachment::Type::eDepth,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

        vk::Subpass subpass;
        subpass.AddDescription(VK_PIPELINE_BIND_POINT_GRAPHICS, attachment);
        subpass.AddDependencies(
            VK_SUBPASS_EXTERNAL,
            0u,
            VkPipelineStageFlagBits(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT),
            VkPipelineStageFlagBits(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT),
            VkAccessFlagBits(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT),
            VkAccessFlagBits(VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT));

        subpass.AddDependencies(
            0u,
            VK_SUBPASS_EXTERNAL,
            VkPipelineStageFlagBits(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT),
            VkPipelineStageFlagBits(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT),
            VkAccessFlagBits(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT),
            VkAccessFlagBits(VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT));

        m_OutlinePass.Create(attachment, subpass, {}, {});
    }

    void SceneRenderer::RenderForward(VkCommandBuffer cmd, std::uint32_t imageIndex, SceneView& view, const SceneCamera& camera,
                                      vk::DescriptorSet& cameraSet, std::span<const MeshDraw> meshes, ecs::Entity outlined) {
        m_ScenePass.UpdateRenderArea({ {}, view.extent });
        m_ScenePass.Begin(cmd, view.sceneFramebuffer);
        SetViewportAndScissor(cmd, view.extent);
        vkCmdSetDepthBias(cmd, 0.0f, 0.0f, 0.0f);

        skybox.Draw(cmd, camera.skyViewProjection);
        m_PbrPipeline.Bind(cmd);
        cameraSet.Bind(cmd, imageIndex);
        DrawMeshes(cmd, meshes, outlined, m_PbrPipeline, m_OutlinedPbrPipeline);
        particles.Draw(cmd, imageIndex, camera.viewProjection, camera.right, camera.up);
        m_ScenePass.End(cmd);
    }

    void SceneRenderer::RenderForwardPlus(VkCommandBuffer cmd, std::uint32_t imageIndex, SceneView& view, const SceneCamera& camera,
                                          vk::DescriptorSet& cameraSet, std::span<const MeshDraw> meshes, ecs::Entity outlined) {
        const auto firstBlended{ std::ranges::find_if(meshes, &MeshDraw::blended) };
        const std::span<const MeshDraw> opaqueMeshes{ meshes.begin(), firstBlended };
        const std::span<const MeshDraw> blendedMeshes{ firstBlended, meshes.end() };

        m_DepthPass.UpdateRenderArea({ {}, view.extent });
        m_DepthPass.Begin(cmd, view.depthFramebuffer);
        SetViewportAndScissor(cmd, view.extent);
        vkCmdSetDepthBias(cmd, 0.0f, 0.0f, 0.0f);
        m_DepthPipeline.Bind(cmd);
        cameraSet.Bind(cmd, imageIndex);
        DrawMeshes(cmd, opaqueMeshes, outlined, m_DepthPipeline, m_OutlinedDepthPipeline);
        m_DepthPass.End(cmd);

        BufferBarrier(cmd, view.lightTiles, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0u, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT);
        m_CullPipeline.Bind(cmd);
        BindSceneSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, imageIndex, cameraSet, view);
        m_CullPipeline.Dispatch(cmd, vk::ComputePipeline::GroupCount(view.extent.width, tileSize), vk::ComputePipeline::GroupCount(view.extent.height, tileSize), 1u);
        BufferBarrier(cmd, view.lightTiles, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT);

        m_ForwardPlusPass.UpdateRenderArea({ {}, view.extent });
        m_ForwardPlusPass.Begin(cmd, view.sceneFramebuffer);
        SetViewportAndScissor(cmd, view.extent);
        skybox.Draw(cmd, camera.skyViewProjection);
        m_TiledPbrPipeline.Bind(cmd);
        BindSceneSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, imageIndex, cameraSet, view);
        const std::uint32_t tilesPerRow{ vk::ComputePipeline::GroupCount(view.extent.width, tileSize) };
        vkCmdPushConstants(cmd, m_SceneLayout, VK_SHADER_STAGE_FRAGMENT_BIT, tilesPerRowOffset, sizeof(tilesPerRow), &tilesPerRow);
        DrawMeshes(cmd, opaqueMeshes, outlined, m_TiledPbrPipeline, m_TiledPbrPipeline);
        m_PbrPipeline.Bind(cmd);
        DrawMeshes(cmd, blendedMeshes, outlined, m_PbrPipeline, m_OutlinedPbrPipeline);
        particles.Draw(cmd, imageIndex, camera.viewProjection, camera.right, camera.up);
        m_ForwardPlusPass.End(cmd);
    }

    void SceneRenderer::RenderDeferred(VkCommandBuffer cmd, std::uint32_t imageIndex, SceneView& view, const SceneCamera& camera,
                                       vk::DescriptorSet& cameraSet, std::span<const MeshDraw> meshes, ecs::Entity outlined) {
        m_GBufferPass.UpdateRenderArea({ {}, view.extent });
        m_GBufferPass.Begin(cmd, view.gBufferFramebuffer);
        SetViewportAndScissor(cmd, view.extent);
        vkCmdSetDepthBias(cmd, 0.0f, 0.0f, 0.0f);
        m_GBufferPipeline.Bind(cmd);
        cameraSet.Bind(cmd, imageIndex);
        DrawMeshes(cmd, meshes, outlined, m_GBufferPipeline, m_OutlinedGBufferPipeline);
        m_GBufferPass.End(cmd);

        m_DeferredPass.UpdateRenderArea({ {}, view.extent });
        m_DeferredPass.Begin(cmd, view.sceneFramebuffer);
        SetViewportAndScissor(cmd, view.extent);
        m_SunPipeline.Bind(cmd);
        BindSceneSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, imageIndex, cameraSet, view);
        vkCmdDraw(cmd, 3u, 1u, 0u, 0u);
        if (lights.GetCount() != 0u) {
            m_LightVolumePipeline.Bind(cmd);
            m_LightVolume.Bind(cmd);
            vkCmdDrawIndexed(cmd, m_LightVolume.GetIndexCount(), lights.GetCount(), 0u, 0, 0u);
        }
        skybox.Draw(cmd, camera.skyViewProjection);
        particles.Draw(cmd, imageIndex, camera.viewProjection, camera.right, camera.up);
        m_DeferredPass.End(cmd);
    }

    void SceneRenderer::BindSceneSets(VkCommandBuffer cmd, VkPipelineBindPoint bindPoint, std::uint32_t imageIndex, const vk::DescriptorSet& cameraSet,
                                      const SceneView& view) {
        const VkDescriptorSet cameraSets[]{ cameraSet.GetSet(0u, imageIndex), cameraSet.GetSet(1u, imageIndex) };
        vkCmdBindDescriptorSets(cmd, bindPoint, m_SceneLayout, 0u, static_cast<std::uint32_t>(std::size(cameraSets)), cameraSets, 0u, nullptr);
        const VkDescriptorSet inputs{ view.inputs.GetSet(0u, 0u) };
        vkCmdBindDescriptorSets(cmd, bindPoint, m_SceneLayout, 3u, 1u, &inputs, 0u, nullptr);
    }

    void SceneRenderer::DrawMeshes(VkCommandBuffer cmd, std::span<const MeshDraw> meshes, ecs::Entity outlined,
                                   vk::GraphicsPipeline& pipeline, vk::GraphicsPipeline& outlinedPipeline) {
        for (const MeshDraw& draw : meshes) {
            vkCmdPushConstants(cmd, m_SceneLayout, VK_SHADER_STAGE_VERTEX_BIT, 0u, sizeof(draw.model), &draw.model);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_SceneLayout, 2u, 1u, &draw.materialSet, 0u, nullptr);
            vkCmdPushConstants(cmd, m_SceneLayout, VK_SHADER_STAGE_FRAGMENT_BIT, materialOffset, sizeof(draw.material), &draw.material);

            const auto id{ static_cast<std::uint64_t>(draw.entity) };
            const std::uint32_t entityId[2]{ static_cast<std::uint32_t>(id), static_cast<std::uint32_t>(id >> 32u) };
            vkCmdPushConstants(cmd, m_SceneLayout, VK_SHADER_STAGE_FRAGMENT_BIT, entityIdOffset, sizeof(entityId), entityId);

            draw.mesh->Bind(cmd);
            if (draw.entity == outlined) {
                outlinedPipeline.Bind(cmd);
                vkCmdDrawIndexed(cmd, draw.mesh->GetIndexCount(), 1u, 0u, 0, 0u);
                pipeline.Bind(cmd);
            } else {
                vkCmdDrawIndexed(cmd, draw.mesh->GetIndexCount(), 1u, 0u, 0, 0u);
            }
        }
    }

    void SceneRenderer::DrawOutline(VkCommandBuffer cmd, SceneView& view, Mesh& mesh, const xmm::Matrix& meshTransform) {
        xmm::Matrix scale{ 1.0f };
        scale.scale(Vector3D{ outlineScale, outlineScale, outlineScale });
        const xmm::Matrix transform{ meshTransform * scale };

        m_OutlinePass.UpdateRenderArea({ {}, view.extent });
        m_OutlinePass.Begin(cmd, view.outlineFramebuffer);
        SetViewportAndScissor(cmd, view.extent);
        m_OutlinePipeline.Bind(cmd);
        vkCmdPushConstants(cmd, m_OutlineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0u, sizeof(transform), &transform);
        vkCmdPushConstants(cmd, m_OutlineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(transform), sizeof(outlineColor), outlineColor);
        mesh.Bind(cmd);
        vkCmdDrawIndexed(cmd, mesh.GetIndexCount(), 1u, 0u, 0, 0u);
        m_OutlinePass.End(cmd);
    }
} // namespace adh
