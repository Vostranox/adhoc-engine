#pragma once
#include "UndoHistory.hpp"

#include <ImGui/imgui.h>
#include <adh/entity.hpp>

#include <cstddef>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace adh {
    class Scene;
    class Window;
    struct RenderSettings;

    enum class GuizmoMode {
        eTranslate,
        eRotate,
        eScale
    };

    struct GuizmoSettings {
        GuizmoMode mode{ GuizmoMode::eTranslate };
        bool isLocal{ true };
        bool snaps{ false };
        float translateStep{ 0.5f };
        float rotateStep{ 15.0f };
        float scaleStep{ 0.1f };
    };

    struct GameViewAspect {
        float GetRatio() const noexcept {
            return width / height;
        }

        float width{ 1.0f };
        float height{ 1.0f };
        int preset{};

        float screenWidth{ 1.0f };
        float screenHeight{ 1.0f };
    };

    struct EditorPaths {
        std::filesystem::path assets;
        std::filesystem::path scenes;
        std::filesystem::path models;
        std::filesystem::path scripts;
        std::filesystem::path textures;
        std::filesystem::path scriptTemplate;
    };

    struct EditInProgress {
        ecs::Entity owner{ ecs::NULL_ENTITY };
        std::string name;
        std::optional<std::size_t> selected;
        ImGuiID item{};
        int submittedFrame{};
        bool changed{};
    };

    struct EditorContext {
        Window* window{};
        Scene* scene{};
        std::filesystem::path scenePath;
        ecs::Entity selectedEntity{ ecs::NULL_ENTITY };
        ecs::Entity renamedEntity{ ecs::NULL_ENTITY };
        GuizmoSettings guizmo;
        GameViewAspect gameAspect;
        EditorPaths paths;
        UndoHistory history;
        std::optional<EditInProgress> edit;
        std::function<void()> actionAfterSaveQuestion;

        RenderSettings* renderSettings{};
        bool* fpsLimit{};
        bool* maximizeOnPlay{};
        bool isPlaying{};
        bool isPaused{};

        std::vector<std::function<void()>> deferredActions;
    };
} // namespace adh
