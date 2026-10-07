#pragma once
#include <string>

namespace adh {
    struct Tag {
        Tag() = default;

        Tag(const std::string& newTag) : tag{ newTag } {
        }

        const char* Get() const noexcept {
            return tag.data();
        }

        void Set(const char* newTag) noexcept {
            tag = newTag;
        }
        std::string tag;
    };
} // namespace adh
