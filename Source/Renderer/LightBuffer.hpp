#pragma once
#include <Math/Math.hpp>
#include <Vulkan/UniformBuffer.hpp>

#include <adh/entity.hpp>

#include <cstddef>
#include <cstdint>
#include <vector>

namespace adh {
    class LightBuffer {
      public:
        static constexpr std::uint32_t maxLights{ 1024u };

      public:
        void Create(std::uint32_t imageCount);

        void Gather(ecs::World& world);

        void Upload(std::uint32_t imageIndex);

        std::uint32_t GetCount() const noexcept;

        const vk::UniformBuffer& GetBuffer() const noexcept;

      private:
        struct GpuLight {
            Vector3D position;
            float range;
            Vector3D color;
            float intensity;
            Vector3D direction;
            std::int32_t type;
            float cosInner;
            float cosOuter;
            float padding[2];
        };

        static_assert(sizeof(GpuLight) == 64u, "A Light of pbr_data.glsl is 64 bytes");

        static constexpr std::size_t countSize{ 16u };

      private:
        vk::UniformBuffer m_Buffer;
        std::vector<GpuLight> m_Lights;
        bool m_HasReportedLimit{};
    };
} // namespace adh
