#include "LightBuffer.hpp"

#include <Event/Event.hpp>
#include <Scene/Components/Light.hpp>
#include <Scene/Components/Transform.hpp>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>

namespace adh {
    void LightBuffer::Create(std::uint32_t imageCount) {
        m_Buffer.Create(nullptr, countSize + maxLights * sizeof(GpuLight), imageCount, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
        m_Lights.reserve(maxLights);
    }

    void LightBuffer::Gather(ecs::World& world) {
        m_Lights.clear();
        world.get_system<Transform, Light>().for_each([&](Transform& transform, Light& light) {
            if (m_Lights.size() == maxLights) {
                if (!m_HasReportedLimit) {
                    m_HasReportedLimit = true;
                    const std::string message{ "The scene has more than " + std::to_string(maxLights) + " lights, only that many are drawn\n" };
                    EventBus().publish<EditorLogEvent>(EditorLogEvent::Type::eError, message.c_str());
                }
                return;
            }

            const float outerAngle{ std::clamp(light.outerAngle, 0.0f, 89.9f) };
            const float innerAngle{ std::clamp(light.innerAngle, 0.0f, outerAngle) };

            GpuLight& gpuLight{ m_Lights.emplace_back() };
            gpuLight.position  = transform.translate;
            gpuLight.range     = std::max(light.range, 0.001f);
            gpuLight.color     = light.color;
            gpuLight.intensity = light.intensity;
            gpuLight.direction = transform.GetForward();
            gpuLight.type      = static_cast<std::int32_t>(light.type);
            gpuLight.cosInner  = std::cos(math::to_radians(innerAngle));
            gpuLight.cosOuter  = std::cos(math::to_radians(outerAngle));
        });
    }

    void LightBuffer::Upload(std::uint32_t imageIndex) {
        std::byte* copy{ static_cast<std::byte*>(m_Buffer.GetMappedPtr()) + m_Buffer.GetDescriptor(imageIndex).offset };
        const std::uint32_t count{ GetCount() };
        std::memcpy(copy, &count, sizeof(count));
        std::memcpy(copy + countSize, m_Lights.data(), m_Lights.size() * sizeof(GpuLight));
    }

    std::uint32_t LightBuffer::GetCount() const noexcept {
        return static_cast<std::uint32_t>(m_Lights.size());
    }

    const vk::UniformBuffer& LightBuffer::GetBuffer() const noexcept {
        return m_Buffer;
    }
} // namespace adh
