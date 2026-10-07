#pragma once
#include <Math/Math.hpp>

namespace adh {
    enum class RenderPath {
        eForward,
        eForwardPlus,
        eDeferred,
    };

    struct RenderSettings {
        RenderPath renderPath{ RenderPath::eForward };
        float bloomStrength{ 0.04f };
        float bloomRadius{ 0.005f };
        float exposure{ 1.0f };
        Vector3D sunPosition{ 6.0f, 20.0f, -10.0f };
        float sunIntensity{ 10.0f };
        int shadowCascades{ 4 };
        float shadowDistance{ 100.0f };
        float shadowSplitLambda{ 0.75f };
        float shadowSoftness{ 0.03f };
        bool showShadowCascades{ false };
    };
} // namespace adh
