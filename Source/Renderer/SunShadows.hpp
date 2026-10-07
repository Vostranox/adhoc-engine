#pragma once
#include "RenderSettings.hpp"

#include <Math/Math.hpp>
#include <Vulkan/Framebuffer.hpp>
#include <Vulkan/GraphicsPipeline.hpp>
#include <Vulkan/Image.hpp>
#include <Vulkan/ImageView.hpp>
#include <Vulkan/PipelineLayout.hpp>
#include <Vulkan/RenderPass.hpp>
#include <Vulkan/Sampler.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace adh {
    struct MeshDraw;
    struct SceneCamera;

    struct SunShadowData {
        xmm::Matrix cascadeLightSpace[4];
        float cascadeEnd[4];
        float cascadeBlendStart[4];
        float cascadeFilterRadius[4];
        float cascadeNormalOffset[4];
        float cameraForward[4];
        std::int32_t cascadeCount;
        std::int32_t showCascades;
    };

    static_assert(offsetof(SunShadowData, cascadeEnd) == 256u && offsetof(SunShadowData, cascadeCount) == 336u,
                  "SunShadowData is laid out as sun_shadow.glsl's SunShadow block");

    struct SunCascades {
        std::array<xmm::Matrix, 4> viewProjection{};
        SunShadowData data{};
    };

    class SunShadows {
      public:
        static constexpr std::uint32_t maxCascades{ 4u };
        static constexpr std::uint32_t mapSize{ 2048u };

      public:
        void Create();

        static SunCascades Fit(const SceneCamera& camera, const RenderSettings& settings) noexcept;

        void Render(VkCommandBuffer cmd, const SunCascades& cascades, std::span<const MeshDraw> meshes);

        VkDescriptorImageInfo GetDescriptor() const noexcept;

      private:
        vk::Image m_Maps;
        std::array<vk::ImageView, maxCascades> m_LayerViews;
        std::array<vk::Framebuffer, maxCascades> m_Framebuffers;
        vk::Sampler m_Sampler;
        vk::RenderPass m_Pass;
        vk::PipelineLayout m_Layout;
        vk::GraphicsPipeline m_Pipeline;
    };
} // namespace adh
