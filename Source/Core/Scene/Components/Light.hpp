#pragma once
#include <Math/Math.hpp>

#include <cstdint>
#include <cstring>

namespace adh {
    struct DirectionalLight {
        alignas(16) Vector3D direction;
        alignas(16) Vector3D color;
        alignas(4) float intensity;
        alignas(4) int castsShadow = false;
        alignas(4) int samplerId   = -1;
    };

    struct Light {
        enum class Type : std::int32_t {
            ePoint,
            eSpot
        };

        const char* GetType() const noexcept;

        bool SetType(const char* name) noexcept;

        Type type{ Type::ePoint };
        Vector3D color{ 1.0f, 1.0f, 1.0f };
        float intensity{ 20.0f };
        float range{ 10.0f };
        float innerAngle{ 20.0f };
        float outerAngle{ 30.0f };
    };

    inline const char* ToString(Light::Type type) noexcept {
        switch (type) {
        case Light::Type::ePoint:
            return "Point";
        case Light::Type::eSpot:
            return "Spot";
        }
        return "Point";
    }

    inline const char* Light::GetType() const noexcept {
        return ToString(type);
    }

    inline bool Light::SetType(const char* name) noexcept {
        if (name == nullptr) {
            return false;
        }
        for (const Type candidate : { Type::ePoint, Type::eSpot }) {
            if (std::strcmp(name, ToString(candidate)) == 0) {
                type = candidate;
                return true;
            }
        }
        return false;
    }
} // namespace adh
