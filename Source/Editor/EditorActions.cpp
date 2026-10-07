#include "EditorActions.hpp"
#include "EditorCamera.hpp"
#include "EditorContext.hpp"
#include "Shell.hpp"

#include <Event/Event.hpp>
#include <Scene/Components.hpp>
#include <Scene/Scene.hpp>
#include <Utf8.hpp>
#include <Window.hpp>

#include <ImGui/imgui_internal.h>
#include <ImGuizmo.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <numbers>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace adh {
    namespace actions {
        static void Defer(EditorContext& context, std::function<void()> action) {
            context.deferredActions.push_back(std::move(action));
        }

        static bool SaveSceneTo(EditorContext& context, const std::filesystem::path& path) {
            FinishEdit(context);
            if (!context.scene->SaveToFile(path)) {
                return false;
            }
            context.scenePath = path;
            context.history.MarkSaved();
            const std::string message{ "Saved " + ToUtf8(path) + "\n" };
            EventBus().publish<EditorLogEvent>(EditorLogEvent::Type::eLog, message.c_str());
            return true;
        }

        static void ShowSaveAsDialog(EditorContext& context, std::function<void()> afterSaving) {
            FinishEdit(context);
            const std::filesystem::path suggested{ context.scenePath.empty() ? context.paths.scenes / PathFromUtf8(context.scene->GetTag() + ".scene")
                                                                             : context.scenePath };
            shell::ShowFileDialog(shell::FileDialog::eSave, suggested, [&context, afterSaving = std::move(afterSaving)](const std::filesystem::path& chosen) {
                if (!CanChangeFiles(context)) {
                    return;
                }
                std::filesystem::path path{ chosen };
                if (!path.has_extension()) {
                    path += ".scene";
                }
                context.scene->SetTag(ToUtf8(path.stem()));
                if (SaveSceneTo(context, path) && afterSaving) {
                    afterSaving();
                }
            });
        }

        static std::string CopyName(ecs::World& world, std::string name) {
            const std::size_t open{ name.rfind(" (") };
            const auto isDigit = [](char c) {
                return std::isdigit(static_cast<unsigned char>(c)) != 0;
            };
            if (open != std::string::npos && name.ends_with(')') && name.size() > open + 3 &&
                std::all_of(name.begin() + static_cast<std::ptrdiff_t>(open) + 2, name.end() - 1, isDigit)) {
                name.erase(open);
            }

            std::unordered_set<std::string> names;
            world.get_system<Tag>().for_each([&](Tag& tag) {
                names.insert(tag.tag);
            });
            for (int number{ 1 };; ++number) {
                std::string copyName{ name + " (" + std::to_string(number) + ")" };
                if (!names.contains(copyName)) {
                    return copyName;
                }
            }
        }

        static std::optional<std::size_t> GetSelectedPlace(EditorContext& context) {
            const std::vector<ecs::Entity> entities{ context.scene->GetEntities() };
            const auto selected{ std::ranges::find(entities, context.selectedEntity) };
            if (selected == entities.end()) {
                return std::nullopt;
            }
            return static_cast<std::size_t>(selected - entities.begin());
        }

        static void RecordEdit(EditorContext& context, std::string name, std::optional<std::size_t> selectedBefore) {
            if (!context.isPlaying) {
                context.history.Record(std::move(name), selectedBefore, context.scene->SaveToText(), GetSelectedPlace(context));
            }
        }

        static void EndEdit(EditorContext& context) {
            if (auto edit{ std::exchange(context.edit, std::nullopt) }) {
                if (edit->changed) {
                    RecordEdit(context, std::move(edit->name), edit->selected);
                }
                if (edit->item != 0 && ImGui::GetActiveID() == edit->item) {
                    ImGui::ClearActiveID();
                }
            }
        }

        static void ObserveEdit(EditorContext& context, ecs::Entity owner, std::string name, ImGuiID item, bool active, bool changed) {
            if (context.edit && (context.edit->owner != owner || context.edit->item != item)) {
                EndEdit(context);
            }
            if (!context.edit && (active || changed)) {
                context.edit = EditInProgress{ owner, std::move(name), GetSelectedPlace(context), item, ImGui::GetFrameCount(), false };
            }
            if (context.edit) {
                context.edit->submittedFrame = ImGui::GetFrameCount();
                context.edit->changed |= changed;
                if (!active) {
                    EndEdit(context);
                }
            }
        }

        static void Restore(EditorContext& context, const SceneSnapshot& snapshot) {
            Scene& scene{ *context.scene };
            std::string name{ scene.GetTag() };
            const Camera3D* sceneCamera{ scene.GetSceneCamera() };
            const std::optional<Camera3D> view{ sceneCamera ? std::optional<Camera3D>{ *sceneCamera } : std::nullopt };

            scene.LoadFromText(snapshot.text);
            scene.SetTag(std::move(name));
            if (Camera3D* camera{ scene.GetSceneCamera() }; camera && view) {
                camera->eyePosition   = view->eyePosition;
                camera->focusPosition = view->focusPosition;
                camera->upVector      = view->upVector;
                camera->aspectRatio   = view->aspectRatio;
            }

            const std::vector<ecs::Entity> entities{ scene.GetEntities() };
            const bool hasSelected{ snapshot.selected && *snapshot.selected < entities.size() };
            context.selectedEntity = hasSelected ? entities[*snapshot.selected] : ecs::NULL_ENTITY;
        }

        static void SaveChangesThen(EditorContext& context, std::function<void()> action) {
            Defer(context, [&context, action = std::move(action)] {
                FinishEdit(context);
                if (HasUnsavedChanges(context)) {
                    context.actionAfterSaveQuestion = action;
                } else {
                    action();
                }
            });
        }

        static void Play(EditorContext& context) {
            Defer(context, [&context] {
                FinishEdit(context);
                EventBus().publish<StatusEvent>(StatusEvent::Type::eRun);
            });
        }

        void Stop(EditorContext& context) {
            Defer(context, [&context] {
                FinishEdit(context);
                EventBus().publish<StatusEvent>(StatusEvent::Type::eStop);
                context.selectedEntity = ecs::NULL_ENTITY;
            });
        }

        void Pause(EditorContext& context) {
            Defer(context, [] {
                EventBus().publish<StatusEvent>(StatusEvent::Type::ePause);
            });
        }

        void Unpause(EditorContext& context) {
            Defer(context, [] {
                EventBus().publish<StatusEvent>(StatusEvent::Type::eUnpause);
            });
        }

        void TogglePlay(EditorContext& context) {
            if (context.isPlaying) {
                Stop(context);
            } else {
                Play(context);
            }
        }

        bool CanChangeFiles(const EditorContext& context) {
            return !context.isPlaying && !shell::IsFileDialogShown();
        }

        void NewScene(EditorContext& context) {
            if (!CanChangeFiles(context)) {
                return;
            }
            SaveChangesThen(context, [&context] {
                Defer(context, [&context] {
                    context.scene->ResetToDefault();
                    context.scenePath.clear();
                    context.selectedEntity = ecs::NULL_ENTITY;
                    context.history.Reset(context.scene->SaveToText());
                });
            });
        }

        void OpenScene(EditorContext& context) {
            if (!CanChangeFiles(context)) {
                return;
            }
            Defer(context, [&context] {
                FinishEdit(context);
                shell::ShowFileDialog(shell::FileDialog::eOpen, context.paths.scenes, [&context](const std::filesystem::path& path) {
                    OpenSceneFile(context, path);
                });
            });
        }

        void OpenSceneFile(EditorContext& context, const std::filesystem::path& path) {
            if (!CanChangeFiles(context)) {
                return;
            }
            SaveChangesThen(context, [&context, path] {
                Defer(context, [&context, path] {
                    if (context.scene->LoadFromFile(path)) {
                        context.scenePath = path;
                        context.history.Reset(context.scene->SaveToText());
                    }
                    context.selectedEntity = ecs::NULL_ENTITY;
                });
            });
        }

        void SaveScene(EditorContext& context) {
            SaveSceneThen(context, {});
        }

        void SaveSceneThen(EditorContext& context, std::function<void()> afterSaving) {
            if (!CanChangeFiles(context)) {
                return;
            }
            Defer(context, [&context, afterSaving = std::move(afterSaving)] {
                if (!CanChangeFiles(context)) {
                    return;
                }
                if (context.scenePath.empty()) {
                    ShowSaveAsDialog(context, afterSaving);
                } else if (SaveSceneTo(context, context.scenePath) && afterSaving) {
                    afterSaving();
                }
            });
        }

        void SaveSceneAs(EditorContext& context) {
            if (CanChangeFiles(context)) {
                Defer(context, [&context] {
                    if (CanChangeFiles(context)) {
                        ShowSaveAsDialog(context, {});
                    }
                });
            }
        }

        void CloseWindow(EditorContext& context) {
            const auto close = [&context] {
                SaveChangesThen(context, [&context] {
                    context.window->SetOpen(false);
                });
            };
            if (context.isPlaying && HasUnsavedChanges(context)) {
                Stop(context);
                Defer(context, close);
            } else {
                close();
            }
        }

        bool HasUnsavedChanges(const EditorContext& context) {
            return context.history.HasUnsavedChanges() || (!context.isPlaying && context.edit && context.edit->changed);
        }

        const char* GetName(EntityKind kind) {
            switch (kind) {
            case EntityKind::eEmpty:
                return "Empty";
            case EntityKind::eCube:
                return "Cube";
            case EntityKind::eSphere:
                return "Sphere";
            case EntityKind::ePlane:
                return "Plane";
            case EntityKind::ePointLight:
                return "Point Light";
            case EntityKind::eSpotLight:
                return "Spot Light";
            case EntityKind::eCamera:
                return "Camera";
            }
            return "Entity";
        }

        void CreateEntity(EditorContext& context, EntityKind kind) {
            Defer(context, [&context, kind] {
                FinishEdit(context);
                const std::optional<std::size_t> selectedBefore{ GetSelectedPlace(context) };
                ecs::World& world{ context.scene->GetWorld() };
                const Camera3D* sceneCamera{ context.scene->GetSceneCamera() };
                const Camera3D view{ sceneCamera ? *sceneCamera : Camera3D{} };
                const Vector3D position{ view.focusPosition };
                constexpr float quarterTurn{ std::numbers::pi_v<float> / 2.0f };
                const Vector3D noRotation{};
                const Vector3D unitScale{ 1.0f, 1.0f, 1.0f };

                const ecs::Entity entity{ world.create_entity() };
                world.add<Tag>(entity, Tag{ GetName(kind) });
                const auto addMesh = [&](const char* model, const Vector3D& rotation) {
                    world.add<Transform, Mesh, Material>(entity, Transform{ position, rotation, unitScale }, Mesh{}, Material{});
                    world.get<Mesh>(entity).Load(ToUtf8(context.paths.models / model));
                };
                switch (kind) {
                case EntityKind::eEmpty:
                    world.add<Transform>(entity, Transform{ position, noRotation, unitScale });
                    break;
                case EntityKind::eCube:
                    addMesh("cube.obj", noRotation);
                    break;
                case EntityKind::eSphere:
                    addMesh("sphere.obj", noRotation);
                    break;
                case EntityKind::ePlane:
                    addMesh("plane.obj", Vector3D{ -quarterTurn, 0.0f, 0.0f });
                    break;
                case EntityKind::ePointLight:
                    world.add<Transform, Light>(entity, Transform{ position, noRotation, unitScale }, Light{});
                    break;
                case EntityKind::eSpotLight:
                    {
                        Light light;
                        light.type = Light::Type::eSpot;
                        world.add<Transform, Light>(entity, Transform{ position, Vector3D{ quarterTurn, 0.0f, 0.0f }, unitScale }, light);
                        break;
                    }
                case EntityKind::eCamera:
                    {
                        Camera3D camera{ view };
                        camera.isSceneCamera   = false;
                        camera.isRuntimeCamera = false;
                        world.add<Camera3D>(entity, camera);
                        break;
                    }
                }
                context.selectedEntity = entity;
                context.renamedEntity  = entity;
                RecordEdit(context, std::string{ "Create " } + GetName(kind), selectedBefore);
            });
        }

        std::string GetEntityName(EditorContext& context, ecs::Entity entity) {
            ecs::World& world{ context.scene->GetWorld() };
            if (world.has_component<Tag>(entity) && !world.get<Tag>(entity).tag.empty()) {
                return world.get<Tag>(entity).tag;
            }
            return "Entity";
        }

        void CommitEdit(EditorContext& context, std::string name) {
            RecordEdit(context, std::move(name), GetSelectedPlace(context));
        }

        FieldsEdit::FieldsEdit(EditorContext& context, ecs::Entity owner, std::string name)
            : m_Context{ context },
              m_Owner{ owner },
              m_Name{ std::move(name) },
              m_ActiveBefore{ GImGui->ActiveIdIsAlive },
              m_DeactivatedBefore{ GImGui->DeactivatedItemData.IsAlive },
              m_EditedBefore{ GImGui->AnyIdHasBeenEditedThisFrame } {
            if (context.edit && context.edit->item != 0 && context.edit->item != ImGui::GetActiveID()) {
                EndEdit(context);
            }
            GImGui->AnyIdHasBeenEditedThisFrame = false;
            ImGui::PushItemFlag(ImGuiItemFlags_LiveEditOnInput, true);
        }

        FieldsEdit::~FieldsEdit() {
            ImGui::PopItemFlag();
            ImGuiContext& gui{ *GImGui };
            const bool active{ gui.ActiveId != 0 && m_ActiveBefore != gui.ActiveId && gui.ActiveIdIsAlive == gui.ActiveId };
            const bool deactivated{ !m_DeactivatedBefore && gui.DeactivatedItemData.IsAlive };
            const bool changed{ gui.AnyIdHasBeenEditedThisFrame };
            gui.AnyIdHasBeenEditedThisFrame |= m_EditedBefore;
            const ImGuiID item{ active ? gui.ActiveId : deactivated ? gui.DeactivatedItemData.ID
                                                                    : 0 };
            if (active || deactivated || changed) {
                ObserveEdit(m_Context, m_Owner, std::move(m_Name), item, active, changed);
            }
        }

        void FinishEdit(EditorContext& context) {
            const bool isGuizmoEdit{ context.edit && context.edit->item == 0 };
            EndEdit(context);
            if (isGuizmoEdit) {
                ImGuizmo::Enable(false);
            }
        }

        void EndEditFrame(EditorContext& context) {
            if (context.edit && (context.edit->submittedFrame != ImGui::GetFrameCount() || (context.edit->item != 0 && context.edit->item != ImGui::GetActiveID()))) {
                EndEdit(context);
            }
        }

        void TrackGuizmoEdit(EditorContext& context, ecs::Entity owner, std::string name, bool changed) {
            ObserveEdit(context, owner, std::move(name), 0, true, changed);
        }

        void SelectEntity(EditorContext& context, ecs::Entity entity) {
            if (context.selectedEntity != entity) {
                FinishEdit(context);
                context.selectedEntity = entity;
                context.renamedEntity  = ecs::NULL_ENTITY;
            }
        }

        bool CanUndo(const EditorContext& context) {
            return !context.isPlaying && context.history.CanUndo();
        }

        bool CanRedo(const EditorContext& context) {
            return !context.isPlaying && context.history.CanRedo();
        }

        void Undo(EditorContext& context) {
            if (context.isPlaying) {
                return;
            }
            Defer(context, [&context] {
                FinishEdit(context);
                if (context.history.CanUndo()) {
                    Restore(context, context.history.Undo());
                }
            });
        }

        void Redo(EditorContext& context) {
            if (context.isPlaying) {
                return;
            }
            Defer(context, [&context] {
                FinishEdit(context);
                if (context.history.CanRedo()) {
                    Restore(context, context.history.Redo());
                }
            });
        }

        void RenameSelected(EditorContext& context) {
            FinishEdit(context);
            context.renamedEntity = context.selectedEntity;
        }

        void DuplicateSelected(EditorContext& context) {
            if (context.selectedEntity == ecs::NULL_ENTITY) {
                return;
            }
            Defer(context, [&context, entity = context.selectedEntity] {
                FinishEdit(context);
                ecs::World& world{ context.scene->GetWorld() };
                if (!world.is_valid(entity)) {
                    return;
                }
                const std::optional<std::size_t> selectedBefore{ GetSelectedPlace(context) };
                const ecs::Entity copy{ context.scene->DuplicateEntity(entity) };
                if (world.has_component<Tag>(copy)) {
                    Tag& tag{ world.get<Tag>(copy) };
                    tag.tag = CopyName(world, tag.tag);
                }
                context.selectedEntity = copy;
                RecordEdit(context, "Duplicate " + GetEntityName(context, entity), selectedBefore);
            });
        }

        void DeleteSelected(EditorContext& context) {
            if (context.selectedEntity == ecs::NULL_ENTITY) {
                return;
            }
            Defer(context, [&context, entity = context.selectedEntity] {
                FinishEdit(context);
                ecs::World& world{ context.scene->GetWorld() };
                if (!world.is_valid(entity)) {
                    return;
                }
                const std::optional<std::size_t> selectedBefore{ GetSelectedPlace(context) };
                std::string name{ "Delete " + GetEntityName(context, entity) };
                world.destroy(entity);
                if (context.selectedEntity == entity) {
                    context.selectedEntity = ecs::NULL_ENTITY;
                }
                RecordEdit(context, std::move(name), selectedBefore);
            });
        }

        void FrameSelected(EditorContext& context) {
            ecs::World& world{ context.scene->GetWorld() };
            Camera3D* camera{ context.scene->GetSceneCamera() };
            if (!camera || !world.has_component<Transform>(context.selectedEntity)) {
                return;
            }
            const Transform& transform{ world.get<Transform>(context.selectedEntity) };
            const Vector3D& scale{ transform.scale };
            EditorCamera::Frame(*camera, transform.translate, 1.75f * std::max({ std::abs(scale.x), std::abs(scale.y), std::abs(scale.z) }));
        }

        void Deselect(EditorContext& context) {
            SelectEntity(context, ecs::NULL_ENTITY);
        }

        void Translate(EditorContext& context) {
            context.guizmo.mode = GuizmoMode::eTranslate;
        }

        void Rotate(EditorContext& context) {
            context.guizmo.mode = GuizmoMode::eRotate;
        }

        void Scale(EditorContext& context) {
            context.guizmo.mode = GuizmoMode::eScale;
        }

        void ToggleGuizmoAxes(EditorContext& context) {
            context.guizmo.isLocal = !context.guizmo.isLocal;
        }

        void RunShortcuts(EditorContext& context, bool keysEditScene) {
            if (ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel)) {
                return;
            }
            const auto run = [&context](const MenuAction& action, ImGuiInputFlags flags = ImGuiInputFlags_RouteGlobal | ImGuiInputFlags_RouteOverActive) {
                if (ImGui::Shortcut(action.shortcut, flags)) {
                    action.run(context);
                }
            };

            for (const MenuAction& action : fileActions) {
                run(action);
            }
            if (ImGui::GetIO().WantTextInput) {
                return;
            }
            run(undoAction);
            run(redoAction);
            run(playAction);
            if (keysEditScene) {
                for (const MenuAction& action : entityActions) {
                    run(action, ImGuiInputFlags_RouteAlways);
                }
                for (const MenuAction& action : selectionActions) {
                    run(action, ImGuiInputFlags_RouteAlways);
                }
                for (const MenuAction& action : { translateAction, rotateAction, scaleAction, guizmoAxesAction }) {
                    run(action, ImGuiInputFlags_RouteAlways);
                }
            }
        }

        void ApplyDeferred(EditorContext& context) {
            while (!context.deferredActions.empty()) {
                for (const std::function<void()>& action : std::exchange(context.deferredActions, {})) {
                    action();
                }
            }
        }
    } // namespace actions
} // namespace adh
