#pragma once
#include <Math/Math.hpp>
#include <Vulkan/GraphicsPipeline.hpp>
#include <Vulkan/PipelineLayout.hpp>
#include <Vulkan/RenderPass.hpp>
#include <Vulkan/UniformBuffer.hpp>

#include <adh/entity.hpp>

#include <cstdint>

namespace adh {
    class ParticleRenderer {
      public:
        static constexpr std::uint32_t maxParticles{ 65'536u };

      public:
        void Create(const vk::RenderPass& scenePass, std::uint32_t imageCount);

        void Upload(ecs::World& world, std::uint32_t imageIndex);

        void Draw(VkCommandBuffer cmd, std::uint32_t imageIndex, const xmm::Matrix& viewProjection, const Vector3D& right, const Vector3D& up);

      private:
        struct Instance {
            Vector3D position;
            float size;
            Vector3D color;
            float alpha;
        };

        static_assert(sizeof(Instance) == 32u, "particle.vert reads 32 bytes per particle");

        struct Camera {
            xmm::Matrix viewProjection;
            Vector4D right;
            Vector4D up;
        };

      private:
        vk::PipelineLayout m_Layout;
        vk::GraphicsPipeline m_Pipeline;
        vk::UniformBuffer m_Instances;
        std::uint32_t m_Count{};
        bool m_HasReportedLimit{};
    };
} // namespace adh
