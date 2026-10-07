#pragma once
#include <Math/Math.hpp>

namespace adh {
    struct Transform {
        Transform() = default;
        Transform(Vector3D newTranslate, Vector3D newRotation, Vector3D newScale)
            : translate{ newTranslate },
              rotation{ newRotation },
              scale{ newScale } {}

        Vector3D translate{};
        Vector3D rotation{};
        Vector3D scale{ 1.0f, 1.0f, 1.0f };

        Vector3D forward{};

        Quaternion<float> q;

        Vector3D& GetForward() noexcept {
            forward = Quaternion<float>{ rotation }.get_vector(Vector3D{ 0.0f, 0.0f, 1.0f });
            return forward;
        }

        Matrix4D GetPhysics() noexcept {
            Matrix4D ret{ 1.0f };
            ret.translate(translate);

            // TODO: Temp
            auto m = q.get_matrix4d();
            Vector3D t, r, s;
            m.decompose(t, r, s);
            rotation = r;
            ret      = ret * m;

            ret.scale(scale);
            return ret;
        }

        xmm::Matrix GetXmmPhysics() noexcept {
            xmm::Matrix ret{ 1.0f };
            ret.translate(translate);

            // TODO: Temp
            auto m = q.get_xmm_matrix();
            Vector3D t, r, s;
            m.decompose(t, r, s);
            rotation = r;
            ret      = ret * m;

            ret.scale(scale);
            return ret;
        }

        Matrix4D Get() noexcept {
            Matrix4D ret{ 1.0f };
            ret.translate(translate);
            ret.rotate(rotation);
            ret.scale(scale);
            return ret;
        }

        xmm::Matrix GetXmm() noexcept {
            xmm::Matrix ret{ 1.0f };
            ret.translate(translate);
            ret.rotate(rotation);
            ret.scale(scale);
            return ret;
        }
    };
} // namespace adh
