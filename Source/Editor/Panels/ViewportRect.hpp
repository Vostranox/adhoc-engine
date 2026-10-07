#pragma once

namespace adh {
    struct ViewportRect {
        bool IsInViewportRect(float x, float y) const noexcept {
            return (x >= left && x <= right && y >= top && y <= bottom);
        }

        float left{};
        float right{};
        float top{};
        float bottom{};
    };

    struct ViewportImage {
        bool isVisible{};
        float width{};
        float height{};
    };
} // namespace adh
