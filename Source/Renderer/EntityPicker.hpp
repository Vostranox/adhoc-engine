#pragma once
#include "SceneRenderer.hpp"

#include <Vulkan/Buffer.hpp>

#include <adh/entity.hpp>

namespace adh {
    class EntityPicker {
      public:
        void Create();

        void Record(VkCommandBuffer cmd, const SceneView& view, float u, float v);

        bool IsPending() const noexcept;

        ecs::Entity Read() noexcept;

      private:
        vk::Buffer m_Buffer;
        void* m_Texel{};
        bool m_IsPending{};
    };
} // namespace adh
