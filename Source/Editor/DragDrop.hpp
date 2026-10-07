#pragma once
#include <filesystem>
#include <initializer_list>
#include <optional>
#include <string_view>

namespace adh {
    void DragAssetPath(const std::filesystem::path& path);

    std::optional<std::filesystem::path> AcceptAssetPath(std::initializer_list<std::string_view> extensions);
} // namespace adh
