#include "Theme.hpp"

#include <Utf8.hpp>

namespace adh {
    namespace theme {
        static Fonts fonts;
        static ImVector<ImWchar> largeIconRanges;

        static ImVec4 WithAlpha(ImVec4 colour, float alpha) {
            colour.w = alpha;
            return colour;
        }

        static ImFont* AddFont(const std::filesystem::path& folder, const char* file, bool withIcons) {
            constexpr float size{ 16.0f };
            ImFontAtlas& atlas{ *ImGui::GetIO().Fonts };
            ImFont* font{ atlas.AddFontFromFileTTF(ToUtf8(folder / file).c_str(), size) };
            if (withIcons) {
                static constexpr ImWchar iconRanges[]{ ICON_MIN_FA, ICON_MAX_FA, 0 };
                ImFontConfig iconConfig;
                iconConfig.MergeMode  = true;
                iconConfig.PixelSnapH = true;
                atlas.AddFontFromFileTTF(ToUtf8(folder / "FontAwesome/fa-solid-900.ttf").c_str(), size, &iconConfig, iconRanges);
            }
            return font;
        }

        void ApplyStyle() {
            ImGuiStyle& style{ ImGui::GetStyle() };
            ImGui::StyleColorsDark(&style);

            ImVec4* colours{ style.Colors };
            colours[ImGuiCol_Text]            = text;
            colours[ImGuiCol_TextDisabled]    = textDim;
            colours[ImGuiCol_TextLink]        = accentHovered;
            colours[ImGuiCol_TextSelectedBg]  = WithAlpha(accent, 0.45f);
            colours[ImGuiCol_InputTextCursor] = text;

            colours[ImGuiCol_WindowBg]     = panel;
            colours[ImGuiCol_ChildBg]      = WithAlpha(panel, 0.0f);
            colours[ImGuiCol_PopupBg]      = popup;
            colours[ImGuiCol_Border]       = border;
            colours[ImGuiCol_BorderShadow] = WithAlpha(border, 0.0f);
            colours[ImGuiCol_MenuBarBg]    = bar;

            colours[ImGuiCol_TitleBg]          = bar;
            colours[ImGuiCol_TitleBgActive]    = bar;
            colours[ImGuiCol_TitleBgCollapsed] = bar;

            colours[ImGuiCol_FrameBg]        = field;
            colours[ImGuiCol_FrameBgHovered] = hovered;
            colours[ImGuiCol_FrameBgActive]  = pressed;

            colours[ImGuiCol_Button]        = control;
            colours[ImGuiCol_ButtonHovered] = hovered;
            colours[ImGuiCol_ButtonActive]  = pressed;

            colours[ImGuiCol_Header]        = control;
            colours[ImGuiCol_HeaderHovered] = hovered;
            colours[ImGuiCol_HeaderActive]  = pressed;

            colours[ImGuiCol_CheckMark]          = accentHovered;
            colours[ImGuiCol_CheckboxSelectedBg] = field;
            colours[ImGuiCol_SliderGrab]         = accent;
            colours[ImGuiCol_SliderGrabActive]   = accentHovered;

            colours[ImGuiCol_Separator]         = border;
            colours[ImGuiCol_SeparatorHovered]  = accentHovered;
            colours[ImGuiCol_SeparatorActive]   = accent;
            colours[ImGuiCol_ResizeGrip]        = WithAlpha(accent, 0.0f);
            colours[ImGuiCol_ResizeGripHovered] = accentHovered;
            colours[ImGuiCol_ResizeGripActive]  = accent;

            colours[ImGuiCol_ScrollbarBg]          = WithAlpha(panel, 0.0f);
            colours[ImGuiCol_ScrollbarGrab]        = control;
            colours[ImGuiCol_ScrollbarGrabHovered] = hovered;
            colours[ImGuiCol_ScrollbarGrabActive]  = pressed;

            colours[ImGuiCol_Tab]                       = bar;
            colours[ImGuiCol_TabHovered]                = hovered;
            colours[ImGuiCol_TabSelected]               = panel;
            colours[ImGuiCol_TabSelectedOverline]       = accent;
            colours[ImGuiCol_TabDimmed]                 = bar;
            colours[ImGuiCol_TabDimmedSelected]         = panel;
            colours[ImGuiCol_TabDimmedSelectedOverline] = pressed;

            colours[ImGuiCol_DockingPreview] = WithAlpha(accent, 0.7f);
            colours[ImGuiCol_DockingEmptyBg] = bar;

            colours[ImGuiCol_TableHeaderBg]     = control;
            colours[ImGuiCol_TableBorderStrong] = border;
            colours[ImGuiCol_TableBorderLight]  = border;
            colours[ImGuiCol_TableRowBg]        = WithAlpha(text, 0.0f);
            colours[ImGuiCol_TableRowBgAlt]     = WithAlpha(text, 0.03f);
            colours[ImGuiCol_TreeLines]         = border;

            colours[ImGuiCol_DragDropTarget]   = accentHovered;
            colours[ImGuiCol_DragDropTargetBg] = WithAlpha(accent, 0.2f);
            colours[ImGuiCol_UnsavedMarker]    = text;
            colours[ImGuiCol_NavCursor]        = accentHovered;
            colours[ImGuiCol_ModalWindowDimBg] = ImVec4{ 0.0f, 0.0f, 0.0f, 0.5f };

            style.WindowPadding    = ImVec2{ 8.0f, 8.0f };
            style.FramePadding     = ImVec2{ 6.0f, 4.0f };
            style.ItemSpacing      = ImVec2{ 8.0f, 4.0f };
            style.ItemInnerSpacing = ImVec2{ 4.0f, 4.0f };
            style.CellPadding      = ImVec2{ 4.0f, 2.0f };
            style.IndentSpacing    = 16.0f;
            style.ScrollbarSize    = 12.0f;
            style.GrabMinSize      = 10.0f;

            style.WindowRounding    = 4.0f;
            style.ChildRounding     = 4.0f;
            style.PopupRounding     = 4.0f;
            style.FrameRounding     = 3.0f;
            style.GrabRounding      = 3.0f;
            style.ScrollbarRounding = 6.0f;
            style.TabRounding       = 3.0f;

            style.WindowBorderSize = 1.0f;
            style.PopupBorderSize  = 1.0f;
            style.ChildBorderSize  = 1.0f;
            style.FrameBorderSize  = 0.0f;
            style.TabBorderSize    = 0.0f;

            style.TabBarOverlineSize               = 2.0f;
            style.TabCloseButtonMinWidthSelected   = 0.0f;
            style.TabCloseButtonMinWidthUnselected = 0.0f;
        }

        void LoadFonts(const std::filesystem::path& folder) {
            fonts.regular = AddFont(folder, "OpenSans/OpenSans-Regular.ttf", true);
            fonts.bold    = AddFont(folder, "OpenSans/OpenSans-Bold.ttf", true);
            ImFontConfig monoConfig;
            monoConfig.SizePixels = 16.0f;
            fonts.mono            = ImGui::GetIO().Fonts->AddFontDefaultVector(&monoConfig);

            ImFontGlyphRangesBuilder rangesBuilder;
            rangesBuilder.AddText(largeIcons);
            rangesBuilder.BuildRanges(&largeIconRanges);
            fonts.largeIcons = ImGui::GetIO().Fonts->AddFontFromFileTTF(ToUtf8(folder / "FontAwesome/fa-solid-900.ttf").c_str(), 48.0f, nullptr, largeIconRanges.Data);

            ImGui::GetIO().FontDefault = fonts.regular;
        }

        const Fonts& GetFonts() {
            return fonts;
        }
    } // namespace theme
} // namespace adh
