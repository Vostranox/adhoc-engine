#pragma once
#include <Math/Math.hpp>

#include <cmath>

namespace adh {
    inline void MakeValidLookAt(const Vector3D& eye, Vector3D& focus, Vector3D& up) noexcept {
        if (!(math::magnitude(focus - eye) > 0.0f)) {
            focus = eye + Vector3D{ 0.0f, 0.0f, 1.0f };
        }
        const Vector3D forward{ math::normalize(focus - eye) };
        if (!(math::magnitude(math::cross(forward, up)) > 0.0f)) {
            up = std::abs(forward.y) < 0.9f ? Vector3D{ 0.0f, 1.0f, 0.0f } : Vector3D{ 0.0f, 0.0f, 1.0f };
        }
    }

    inline Matrix4D LookAt(const Vector3D& eye, Vector3D focus, Vector3D up) noexcept {
        MakeValidLookAt(eye, focus, up);
        return math::look_at_lh(eye, focus, up);
    }

    inline xmm::Matrix XmmLookAt(const Vector3D& eye, Vector3D focus, Vector3D up) noexcept {
        MakeValidLookAt(eye, focus, up);
        return xmm::look_at_lh(eye, focus, up);
    }

    struct Camera2D {
        Camera2D() = default;

        Camera2D(const Vector3D& newEyePosition, const Vector3D& newFocusPosition, const Vector3D& newUpVector,
                 float newLeft, float newRight, float newBottom, float newTop, float newNearZ, float newFarZ)
            : eyePosition{ newEyePosition },
              focusPosition{ newFocusPosition },
              upVector{ newUpVector },
              left{ newLeft },
              right{ newRight },
              bottom{ newBottom },
              top{ newTop },
              nearZ{ newNearZ },
              farZ{ newFarZ } {}

        Matrix4D GetView() const noexcept {
            return LookAt(eyePosition, focusPosition, upVector);
        }

        xmm::Matrix GetXmmView() const noexcept {
            return XmmLookAt(eyePosition, focusPosition, upVector);
        }

        Matrix4D GetProjection() {
            return math::orthographic_lh(left, right, bottom, top, nearZ, farZ);
        }

        xmm::Matrix GetXmmProjection() {
            return xmm::orthographic_lh(left, right, bottom, top, nearZ, farZ);
        }

        Vector3D eyePosition{ 0.0f, 0.0f, 0.0f };
        Vector3D focusPosition{ 0.0f, 0.0f, 1.0f };
        Vector3D upVector{ 0.0f, 1.0f, 0.0f };

        float left{ 0.0f };
        float right{ 1.0f };
        float bottom{ 0.0f };
        float top{ 1.0f };
        float nearZ{ 1.0f };
        float farZ{ 100.0f };

        bool isSceneCamera{ false };
        bool isRuntimeCamera{ false };
    };

    struct Camera3D {
        Camera3D() = default;

        Camera3D(const Vector3D& newEyePosition, const Vector3D& newFocusPosition, const Vector3D& newUpVector,
                 float newFieldOfView, float newAspectRatio, float newNearZ, float newFarZ)
            : eyePosition{ newEyePosition },
              focusPosition{ newFocusPosition },
              upVector{ newUpVector },
              fieldOfView{ newFieldOfView },
              aspectRatio{ newAspectRatio },
              nearZ{ newNearZ },
              farZ{ newFarZ } {}

        Matrix4D GetView() const noexcept {
            return LookAt(eyePosition, focusPosition, upVector);
        }

        xmm::Matrix GetXmmView() const noexcept {
            return XmmLookAt(eyePosition, focusPosition, upVector);
        }

        Matrix4D GetProjection() {
            return math::perspective_lh(math::to_radians(fieldOfView), aspectRatio, nearZ, farZ);
        }

        xmm::Matrix GetXmmProjection() {
            return xmm::perspective_lh(math::to_radians(fieldOfView), aspectRatio, nearZ, farZ);
        }

        Vector3D eyePosition{ 0.0f, 0.0f, 0.0f };
        Vector3D focusPosition{ 0.0f, 0.0f, 1.0f };
        Vector3D upVector{ 0.0f, 1.0f, 0.0f };

        float fieldOfView{ 45.0f };
        float aspectRatio{ 1.0f };
        float nearZ{ 1.0f };
        float farZ{ 100.0f };

        bool isSceneCamera{ false };
        bool isRuntimeCamera{ false };
    };
} // namespace adh
