#pragma once
#include "../EditorActions.hpp"

#include <span>

namespace adh {
    class Panel;
    struct EditorContext;

    struct MainMenuBarResult {
        bool resetLayout{ false };
        bool hasKeyboard{ false };
    };

    MainMenuBarResult DrawMainMenuBar(EditorContext& context, std::span<Panel* const> panels);

    void DrawMenuItems(EditorContext& context, std::span<const actions::MenuAction> items, bool isEnabled = true);

    void DrawCreateEntityItems(EditorContext& context);
} // namespace adh
