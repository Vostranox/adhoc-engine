#pragma once
#include <ImGui/imgui.h>
#include <adh/entity.hpp>

#include <filesystem>
#include <functional>
#include <string>

namespace adh {
    struct EditorContext;

    namespace actions {
        void Stop(EditorContext& context);

        void Pause(EditorContext& context);

        void Unpause(EditorContext& context);

        void TogglePlay(EditorContext& context);

        bool CanChangeFiles(const EditorContext& context);

        void NewScene(EditorContext& context);

        void OpenScene(EditorContext& context);

        void OpenSceneFile(EditorContext& context, const std::filesystem::path& path);

        void SaveScene(EditorContext& context);

        void SaveSceneThen(EditorContext& context, std::function<void()> afterSaving);

        void SaveSceneAs(EditorContext& context);

        void CloseWindow(EditorContext& context);

        bool HasUnsavedChanges(const EditorContext& context);

        enum class EntityKind {
            eEmpty,
            eCube,
            eSphere,
            ePlane,
            ePointLight,
            eSpotLight,
            eCamera
        };

        const char* GetName(EntityKind kind);

        void CreateEntity(EditorContext& context, EntityKind kind);

        std::string GetEntityName(EditorContext& context, ecs::Entity entity);

        void CommitEdit(EditorContext& context, std::string name);

        class FieldsEdit {
          public:
            FieldsEdit(EditorContext& context, ecs::Entity owner, std::string name);

            FieldsEdit(const FieldsEdit& rhs) = delete;

            FieldsEdit& operator=(const FieldsEdit& rhs) = delete;

            ~FieldsEdit();

          private:
            EditorContext& m_Context;
            ecs::Entity m_Owner;
            std::string m_Name;
            ImGuiID m_ActiveBefore;
            bool m_DeactivatedBefore;
            bool m_EditedBefore;
        };

        void FinishEdit(EditorContext& context);

        void EndEditFrame(EditorContext& context);

        void TrackGuizmoEdit(EditorContext& context, ecs::Entity owner, std::string name, bool changed);

        void SelectEntity(EditorContext& context, ecs::Entity entity);

        bool CanUndo(const EditorContext& context);

        bool CanRedo(const EditorContext& context);

        void Undo(EditorContext& context);

        void Redo(EditorContext& context);

        void RenameSelected(EditorContext& context);

        void DuplicateSelected(EditorContext& context);

        void DeleteSelected(EditorContext& context);

        void FrameSelected(EditorContext& context);

        void Deselect(EditorContext& context);

        void Translate(EditorContext& context);

        void Rotate(EditorContext& context);

        void Scale(EditorContext& context);

        void ToggleGuizmoAxes(EditorContext& context);

        struct MenuAction {
            const char* label;
            ImGuiKeyChord shortcut;
            void (*run)(EditorContext& context);
        };

        inline constexpr MenuAction fileActions[]{
            { "New", ImGuiMod_Ctrl | ImGuiKey_N, NewScene },
            { "Open...", ImGuiMod_Ctrl | ImGuiKey_O, OpenScene },
            { "Save", ImGuiMod_Ctrl | ImGuiKey_S, SaveScene },
            { "Save As...", ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_S, SaveSceneAs },
        };

        inline constexpr MenuAction undoAction{ "Undo", ImGuiMod_Ctrl | ImGuiKey_Z, Undo };
        inline constexpr MenuAction redoAction{ "Redo", ImGuiMod_Ctrl | ImGuiKey_Y, Redo };

        inline constexpr MenuAction entityActions[]{
            { "Rename", ImGuiKey_F2, RenameSelected },
            { "Duplicate", ImGuiMod_Ctrl | ImGuiKey_D, DuplicateSelected },
            { "Delete", ImGuiKey_Delete, DeleteSelected },
        };
        inline constexpr MenuAction selectionActions[]{
            { "Frame Selected", ImGuiKey_F, FrameSelected },
            { "Deselect", ImGuiKey_Escape, Deselect },
        };

        inline constexpr MenuAction translateAction{ "Translate", ImGuiKey_W, Translate };
        inline constexpr MenuAction rotateAction{ "Rotate", ImGuiKey_E, Rotate };
        inline constexpr MenuAction scaleAction{ "Scale", ImGuiKey_R, Scale };
        inline constexpr MenuAction guizmoAxesAction{ "Move along the entity's axes or the world's", ImGuiKey_X, ToggleGuizmoAxes };
        inline constexpr MenuAction playAction{ "Play or stop", ImGuiMod_Ctrl | ImGuiKey_P, TogglePlay };

        void RunShortcuts(EditorContext& context, bool keysEditScene);

        void ApplyDeferred(EditorContext& context);
    } // namespace actions
} // namespace adh
