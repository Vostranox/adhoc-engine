#pragma once
#include <adh/entity.hpp>

#include <random>

namespace adh {
    class ParticleSystem {
      public:
        void Reset() noexcept;

        void Update(ecs::World& world, float deltaTime);

      private:
        std::mt19937 m_Random;
    };
} // namespace adh
