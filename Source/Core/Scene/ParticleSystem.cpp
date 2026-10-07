#include "ParticleSystem.hpp"
#include "Components/ParticleEmitter.hpp"
#include "Components/Transform.hpp"

#include <algorithm>
#include <cmath>

namespace adh {
    void ParticleSystem::Reset() noexcept {
        m_Random.seed(std::mt19937::default_seed);
    }

    void ParticleSystem::Update(ecs::World& world, float deltaTime) {
        world.get_system<ParticleEmitter>().for_each([&](ecs::Entity entity, ParticleEmitter& emitter) {
            auto& particles{ emitter.particles };

            for (std::size_t i{}; i < particles.size();) {
                ParticleEmitter::Particle& particle{ particles[i] };
                particle.age += deltaTime;
                if (particle.age >= emitter.lifetime) {
                    particle = particles.back();
                    particles.pop_back();
                } else {
                    particle.velocity += emitter.gravity * deltaTime;
                    particle.position += particle.velocity * deltaTime;
                    ++i;
                }
            }

            if (!world.has_component<Transform>(entity)) {
                return;
            }
            const Vector3D origin{ world.get<Transform>(entity).translate };

            constexpr auto limit{ static_cast<float>(ParticleEmitter::particleLimit) };
            emitter.spawnAccumulator += emitter.spawnRate > 0.0f ? emitter.spawnRate * deltaTime : 0.0f;
            if (!std::isfinite(emitter.spawnAccumulator) || emitter.spawnAccumulator > limit) {
                emitter.spawnAccumulator = limit;
            }
            const float due{ std::floor(emitter.spawnAccumulator) };
            emitter.spawnAccumulator -= due;
            const std::size_t maxParticles{ std::min(emitter.maxParticles, ParticleEmitter::particleLimit) };
            const std::size_t room{ maxParticles > particles.size() ? maxParticles - particles.size() : 0u };
            const auto count{ static_cast<std::size_t>(std::min(due, static_cast<float>(room))) };

            const float spread{ std::max(emitter.spread, 0.0f) };
            std::uniform_real_distribution<float> offset{ -spread, spread };
            for (std::size_t i{}; i != count; ++i) {
                const Vector3D velocity{ emitter.velocity.x + offset(m_Random), emitter.velocity.y + offset(m_Random), emitter.velocity.z + offset(m_Random) };
                particles.push_back({ origin, velocity, 0.0f });
            }
        });
    }
} // namespace adh
