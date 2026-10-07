#pragma once
#include <Math/Math.hpp>

#include <cstdint>
#include <vector>

namespace adh {
    struct ParticleEmitter {
        struct Particle {
            Vector3D position;
            Vector3D velocity;
            float age{};
        };

        static constexpr std::uint32_t particleLimit{ 65'536u };

        std::uint32_t maxParticles{ 500u };
        float spawnRate{ 100.0f };
        float lifetime{ 2.0f };
        Vector3D velocity{ 0.0f, 5.0f, 0.0f };
        float spread{ 1.0f };
        Vector3D gravity{ 0.0f, -9.81f, 0.0f };
        float size{ 0.1f };
        Vector3D color{ 1.0f, 0.5f, 0.1f };
        float intensity{ 5.0f };

        std::vector<Particle> particles;
        float spawnAccumulator{};
    };
} // namespace adh
