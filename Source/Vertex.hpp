#pragma once
#include <Math/Math.hpp>

namespace adh {
    struct Vertex {
        Vertex() = default;
        Vertex(const Vector3D& pos, const Vector3D& n, const Vector2D& t, const Vector4D& tan)
            : position{ pos },
              normals{ n },
              textureCoords{ t },
              tangent{ tan } {}
        Vector3D position;
        Vector3D normals;
        Vector2D textureCoords;
        Vector4D tangent;
    };
} // namespace adh
