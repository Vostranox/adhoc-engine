#pragma once
#include "IconFontCppHeaders/IconFontAwesome5.hpp"

#include <ImGui/imgui.h>

#include <filesystem>

namespace adh {
    namespace theme {
        inline constexpr ImVec4 bar{ 0.1f, 0.1f, 0.1f, 1.0f };
        inline constexpr ImVec4 panel{ 0.122f, 0.122f, 0.122f, 1.0f };
        inline constexpr ImVec4 popup{ 0.15f, 0.15f, 0.15f, 1.0f };
        inline constexpr ImVec4 field{ 0.18f, 0.18f, 0.18f, 1.0f };
        inline constexpr ImVec4 control{ 0.22f, 0.22f, 0.22f, 1.0f };
        inline constexpr ImVec4 hovered{ 0.27f, 0.27f, 0.27f, 1.0f };
        inline constexpr ImVec4 pressed{ 0.31f, 0.31f, 0.31f, 1.0f };
        inline constexpr ImVec4 border{ 0.2f, 0.2f, 0.2f, 1.0f };
        inline constexpr ImVec4 text{ 0.9f, 0.9f, 0.9f, 1.0f };
        inline constexpr ImVec4 textDim{ 0.55f, 0.55f, 0.55f, 1.0f };
        inline constexpr ImVec4 folderIcon{ 0.72f, 0.72f, 0.72f, 1.0f };

        inline constexpr ImVec4 accent{ 0.20f, 0.44f, 0.76f, 1.0f };
        inline constexpr ImVec4 accentHovered{ 0.26f, 0.52f, 0.86f, 1.0f };
        inline constexpr ImVec4 accentActive{ 0.16f, 0.36f, 0.64f, 1.0f };

        inline constexpr ImVec4 playingBar{ 0.12f, 0.22f, 0.36f, 1.0f };

        inline constexpr ImVec4 error{ 0.94f, 0.38f, 0.36f, 1.0f };

        inline constexpr ImVec4 axisX{ 0.70f, 0.20f, 0.18f, 1.0f };
        inline constexpr ImVec4 axisY{ 0.28f, 0.55f, 0.16f, 1.0f };
        inline constexpr ImVec4 axisZ{ 0.20f, 0.38f, 0.76f, 1.0f };

        inline constexpr const char* largeIcons{ ICON_FA_FOLDER ICON_FA_FILE ICON_FA_FILE_CODE ICON_FA_FILE_IMAGE ICON_FA_FILE_AUDIO
                                                     ICON_FA_CUBE ICON_FA_CUBES ICON_FA_FONT };

        struct Fonts {
            ImFont* regular{};
            ImFont* bold{};
            ImFont* mono{};
            ImFont* largeIcons{};
        };

        void ApplyStyle();

        void LoadFonts(const std::filesystem::path& folder);

        const Fonts& GetFonts();
    } // namespace theme
} // namespace adh
