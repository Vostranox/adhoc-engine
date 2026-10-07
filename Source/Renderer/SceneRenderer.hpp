#pragma once
#include "Bloom.hpp"
#include "LightBuffer.hpp"
#include "ParticleRenderer.hpp"
#include "RenderSettings.hpp"
#include "SkyboxRenderer.hpp"
#include "SunShadows.hpp"

#include <Math/Math.hpp>
#include <Scene/Components/Material.hpp>
#include <Scene/Components/Mesh.hpp>
#include <Vulkan/Buffer.hpp>
#include <Vulkan/ComputePipeline.hpp>
#include <Vulkan/DescriptorSet.hpp>
#include <Vulkan/Framebuffer.hpp>
#include <Vulkan/GraphicsPipeline.hpp>
#include <Vulkan/Image.hpp>
#include <Vulkan/ImageView.hpp>
#include <Vulkan/PipelineLayout.hpp>
#include <Vulkan/RenderPass.hpp>
#include <Vulkan/Sampler.hpp>

#include <adh/entity.hpp>

#include <span>

namespace adh {
    struct MeshDraw {
        Mesh* mesh;
        xmm::Matrix model;
        Material material;
        VkDescriptorSet materialSet;
        ecs::Entity entity;
        bool blended;
    };

    struct SceneCamera {
        xmm::Matrix viewProjection{ 1.0f };
        xmm::Matrix skyViewProjection{ 1.0f };
        Vector3D right;
        Vector3D up;
        xmm::Matrix view{ 1.0f };
        xmm::Matrix projection{ 1.0f };
    };

    struct SceneView {
        VkExtent2D extent{};
        vk::Image hdr;
        vk::Image depth;
        vk::ImageView depthOnly;
        vk::Image entityIds;
        vk::Framebuffer sceneFramebuffer;
        vk::Framebuffer depthFramebuffer;
        vk::Buffer lightTiles;
        vk::Image gAlbedoMetallic;
        vk::Image gNormalRoughness;
        vk::Image gDepth;
        vk::Image gVertexNormal;
        vk::Framebuffer gBufferFramebuffer;
        vk::DescriptorSet inputs;
        BloomTargets bloom;
        vk::Image ldr;
        vk::Framebuffer ldrFramebuffer;
        vk::Framebuffer outlineFramebuffer;
        vk::DescriptorSet tonemapInputs;
        vk::DescriptorSet presentInput;
    };

    class SceneRenderer {
      public:
        static constexpr std::uint32_t minViewSize{ 128u };

      public:
        void Create(const vk::RenderPass& presentPass, const vk::Sampler& sampler, std::uint32_t imageCount);

        const vk::PipelineLayout& GetSceneLayout() const noexcept;

        void Resize(SceneView& view, VkExtent2D extent);

        void Render(VkCommandBuffer cmd, std::uint32_t imageIndex, SceneView& view, const SceneCamera& camera, const SunCascades& cascades,
                    vk::DescriptorSet& cameraSet, std::span<const MeshDraw> meshes, ecs::Entity outlined);

        void Present(VkCommandBuffer cmd, std::uint32_t imageIndex, SceneView& view);

      private:
        void CreateSceneLayout();

        void CreateLitPass(vk::RenderPass& pass, bool depthDrawn, bool colorDrawn);

        void CreateDepthPass();

        void CreateForwardPlus();

        void CreateGBufferPass();

        void CreateDeferred();

        void CreateOutlinePass();

        void RenderForward(VkCommandBuffer cmd, std::uint32_t imageIndex, SceneView& view, const SceneCamera& camera,
                           vk::DescriptorSet& cameraSet, std::span<const MeshDraw> meshes, ecs::Entity outlined);

        void RenderForwardPlus(VkCommandBuffer cmd, std::uint32_t imageIndex, SceneView& view, const SceneCamera& camera,
                               vk::DescriptorSet& cameraSet, std::span<const MeshDraw> meshes, ecs::Entity outlined);

        void RenderDeferred(VkCommandBuffer cmd, std::uint32_t imageIndex, SceneView& view, const SceneCamera& camera,
                            vk::DescriptorSet& cameraSet, std::span<const MeshDraw> meshes, ecs::Entity outlined);

        void BindSceneSets(VkCommandBuffer cmd, VkPipelineBindPoint bindPoint, std::uint32_t imageIndex, const vk::DescriptorSet& cameraSet,
                           const SceneView& view);

        void DrawMeshes(VkCommandBuffer cmd, std::span<const MeshDraw> meshes, ecs::Entity outlined,
                        vk::GraphicsPipeline& pipeline, vk::GraphicsPipeline& outlinedPipeline);

        void DrawOutline(VkCommandBuffer cmd, SceneView& view, Mesh& mesh, const xmm::Matrix& meshTransform);

      public:
        LightBuffer lights;
        SkyboxRenderer skybox;
        ParticleRenderer particles;
        SunShadows shadows;
        RenderSettings settings;

      private:
        const vk::Sampler* m_Sampler{};
        vk::Sampler m_NearestSampler;
        std::uint32_t m_ImageCount{};
        VkFormat m_DepthFormat{};
        Bloom m_Bloom;

        vk::PipelineLayout m_SceneLayout;

        vk::RenderPass m_ScenePass;
        vk::GraphicsPipeline m_PbrPipeline;
        vk::GraphicsPipeline m_OutlinedPbrPipeline;

        vk::RenderPass m_DepthPass;
        vk::GraphicsPipeline m_DepthPipeline;
        vk::GraphicsPipeline m_OutlinedDepthPipeline;
        vk::ComputePipeline m_CullPipeline;
        vk::RenderPass m_ForwardPlusPass;
        vk::GraphicsPipeline m_TiledPbrPipeline;

        vk::RenderPass m_GBufferPass;
        vk::GraphicsPipeline m_GBufferPipeline;
        vk::GraphicsPipeline m_OutlinedGBufferPipeline;
        vk::RenderPass m_DeferredPass;
        vk::GraphicsPipeline m_SunPipeline;
        vk::GraphicsPipeline m_LightVolumePipeline;
        Mesh m_LightVolume;

        vk::RenderPass m_LdrPass;
        vk::PipelineLayout m_TonemapLayout;
        vk::GraphicsPipeline m_TonemapPipeline;

        vk::PipelineLayout m_PresentLayout;
        vk::GraphicsPipeline m_PresentPipeline;

        vk::RenderPass m_OutlinePass;
        vk::PipelineLayout m_OutlineLayout;
        vk::GraphicsPipeline m_OutlinePipeline;
    };
} // namespace adh
