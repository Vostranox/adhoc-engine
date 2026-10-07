#pragma once
#include <Math/Math.hpp>

namespace adh {
    struct Camera3D;

    class EditorCamera {
      public:
        void Update(Camera3D& camera, bool hovered, float viewHeight) noexcept;

        static void Frame(Camera3D& camera, const Vector3D& center, float radius) noexcept;

        bool IsNavigating() const noexcept;

      private:
        int m_DragButton{ -1 };
        float m_FlySpeed{ 5.0f };
    };
} // namespace adh
