#pragma once
#include <string>

namespace adh {
    struct Skybox {
        static constexpr const char* faceNames[6]{ "right", "left", "top", "bottom", "front", "back" };

        std::string folder{ "Skybox" };
        float intensity{ 1.0f };
    };
} // namespace adh
