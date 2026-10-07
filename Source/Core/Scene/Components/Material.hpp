#pragma once
#include <Math/Math.hpp>

namespace adh {
    struct Material {
        alignas(4) float roughness        = 0.6f;
        alignas(4) float metallicness     = 0.0f;
        alignas(4) float transparency     = 1.0f;
        alignas(4) float heightScale      = 0.0f;
        alignas(16) Vector3D albedo       = { 1.0f, 1.0f, 1.0f };
        alignas(4) float emissive         = 0.0f;
        alignas(4) float ambientOcclusion = 1.0f;
    };
} // namespace adh
