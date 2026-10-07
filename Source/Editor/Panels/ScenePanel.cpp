#include "ScenePanel.hpp"
#include "../DragDrop.hpp"
#include "../EditorActions.hpp"
#include "../EditorContext.hpp"

#include <Scene/Components.hpp>
#include <Scene/Scene.hpp>

#include <ImGui/imgui.h>
#include <ImGuizmo.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>
#include <string>
#include <utility>

namespace adh {
    static float Unwrap(float angle, float previous) noexcept {
        constexpr float twoPi{ 2.0f * std::numbers::pi_v<float> };
        return angle + twoPi * std::round((previous - angle) / twoPi);
    }

    static Vector3D ClosestEquivalentRotation(const Vector3D& r, const Vector3D& previous) noexcept {
        constexpr float pi{ std::numbers::pi_v<float> };
        const Vector3D candidates[2]{ r, Vector3D{ r.x + pi, pi - r.y, r.z + pi } };
        Vector3D best{ r };
        float bestDistance{ std::numeric_limits<float>::infinity() };
        for (const Vector3D& candidate : candidates) {
            const Vector3D unwrapped{ Unwrap(candidate.x, previous.x), Unwrap(candidate.y, previous.y), Unwrap(candidate.z, previous.z) };
            const float distance{ math::magnitude(unwrapped - previous) };
            if (distance < bestDistance) {
                best         = unwrapped;
                bestDistance = distance;
            }
        }
        return best;
    }

    static bool Differs(const Vector3D& a, const Vector3D& b) noexcept {
        constexpr float tolerance{ 1e-4f };
        for (std::size_t i{}; i != 3; ++i) {
            if (std::abs(a[i] - b[i]) > tolerance * std::max(1.0f, std::abs(b[i]))) {
                return true;
            }
        }
        return false;
    }

    static bool Differs(const Transform& a, const Transform& b) noexcept {
        return Differs(a.translate, b.translate) || Differs(a.rotation, b.rotation) || Differs(a.scale, b.scale);
    }

    ScenePanel::ScenePanel() noexcept : ViewportPanel{ "Scene" } {
    }

    void ScenePanel::OnImGui(EditorContext& context) {
        Scene* scene{ context.scene };
        const ecs::Entity entity{ context.selectedEntity };
        const GuizmoSettings& guizmo{ context.guizmo };

        if (m_GuizmoDrag && (!context.edit || context.edit->item != 0 || context.edit->owner != m_GuizmoDrag->entity)) {
            m_GuizmoDrag.reset();
        }
        pick.reset();
        const ImVec2 imageSize{ DrawImage() };
        const bool imageHovered{ ImGui::IsItemHovered(ImGuiHoveredFlags_NoNavOverride) };
        const ImVec2 imageMin{ ImGui::GetItemRectMin() };

        Transform* transform = nullptr;
        xmm::Matrix entityTransform;
        if (scene->GetWorld().has_component<Transform>(entity)) {
            auto& t         = scene->GetWorld().get<Transform>(entity);
            entityTransform = t.GetXmm();
            transform       = &t;
        }

        Camera3D* camera{ scene->GetSceneCamera() };

        xmm::Matrix cameraView;
        xmm::Matrix cameraProj;
        if (camera) {
            if (imageSize.x > 0.0f && imageSize.y > 0.0f) {
                camera->aspectRatio = imageSize.x / imageSize.y;
            }

            m_Camera.Update(*camera, imageHovered, imageSize.y);

            cameraView = camera->GetXmmView();
            cameraProj = camera->GetXmmProjection();
        }

        // Guizmo begin
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(rect.left, rect.top, imageSize.x, imageSize.y);
        ImGuizmo::Enable(!m_Camera.IsNavigating() && !ImGui::GetIO().KeyAlt);

        ImGuizmo::OPERATION operation{ ImGuizmo::OPERATION::TRANSLATE };
        float snap[3]{};
        switch (guizmo.mode) {
        case GuizmoMode::eTranslate:
            {
                operation = ImGuizmo::OPERATION::TRANSLATE;
                snap[0] = snap[1] = snap[2] = guizmo.translateStep;
                break;
            }
        case GuizmoMode::eRotate:
            {
                operation = ImGuizmo::OPERATION::ROTATE;
                snap[0]   = guizmo.rotateStep;
                break;
            }
        case GuizmoMode::eScale:
            {
                operation = ImGuizmo::OPERATION::SCALE;
                snap[0]   = guizmo.scaleStep;
                break;
            }
        }

        ImGuizmo::Manipulate(cameraView.f, cameraProj.f, operation, guizmo.isLocal ? ImGuizmo::LOCAL : ImGuizmo::WORLD, entityTransform.f,
                             nullptr, guizmo.snaps ? snap : nullptr);
        if (ImGuizmo::IsUsing()) {
            if (transform) {
                if (!m_GuizmoDrag) {
                    actions::FinishEdit(context);
                    m_GuizmoDrag = GuizmoDrag{ entity, *transform };
                }
                Vector3D t, r, s;
                entityTransform.decompose(t, r, s);
                switch (guizmo.mode) {
                case GuizmoMode::eTranslate:
                    transform->translate = t;
                    break;
                case GuizmoMode::eRotate:
                    transform->rotation = ClosestEquivalentRotation(r, transform->rotation);
                    break;
                case GuizmoMode::eScale:
                    transform->scale = s;
                    break;
                }
                const char* name{ guizmo.mode == GuizmoMode::eTranslate ? "Move " : guizmo.mode == GuizmoMode::eRotate ? "Rotate "
                                                                                                                       : "Scale " };
                const bool changed{ Differs(*transform, m_GuizmoDrag->start) };
                actions::TrackGuizmoEdit(context, entity, name + actions::GetEntityName(context, entity), changed);
            }
        } else if (m_GuizmoDrag) {
            CommitGuizmoDrag(context);
        }
        // End guizmo

        const bool overGuizmo{ ImGuizmo::IsUsingAny() || ImGuizmo::GetHoveredHandleType() != ImGuizmo::MT_NONE };
        if (imageHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !overGuizmo && !m_Camera.IsNavigating()) {
            const ImVec2 mouse{ ImGui::GetIO().MousePos };
            pick = ScenePick{ (mouse.x - imageMin.x) / imageSize.x, (mouse.y - imageMin.y) / imageSize.y };
        }

        if (actions::CanChangeFiles(context)) {
            if (const auto path{ AcceptAssetPath({ ".scene" }) }) {
                actions::OpenSceneFile(context, *path);
            }
        }
    }

    bool ScenePanel::IsMovingCamera() const noexcept {
        return m_Camera.IsNavigating();
    }

    void ScenePanel::OnHidden(EditorContext& context) {
        ViewportPanel::OnHidden(context);
        pick.reset();
        if (m_GuizmoDrag) {
            CommitGuizmoDrag(context);
        }
    }

    void ScenePanel::CommitGuizmoDrag(EditorContext& context) {
        const GuizmoDrag drag{ *std::exchange(m_GuizmoDrag, std::nullopt) };
        ecs::World& world{ context.scene->GetWorld() };
        if (!world.has_component<Transform>(drag.entity)) {
            return;
        }
        Transform& transform{ world.get<Transform>(drag.entity) };
        if (!Differs(transform, drag.start)) {
            transform.translate = drag.start.translate;
            transform.rotation  = drag.start.rotation;
            transform.scale     = drag.start.scale;
        }
        actions::FinishEdit(context);
    }
} // namespace adh
