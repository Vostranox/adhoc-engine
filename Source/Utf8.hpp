#pragma once
#include <filesystem>
#include <string>
#include <string_view>

namespace adh {
    inline std::string ToUtf8(const std::filesystem::path& path) {
        const std::u8string text{ path.generic_u8string() };
        return std::string{ text.begin(), text.end() };
    }

    inline std::filesystem::path PathFromUtf8(std::string_view text) {
        return std::filesystem::path{ std::u8string{ text.begin(), text.end() } };
    }
} // namespace adh
