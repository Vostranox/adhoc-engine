#include "EditorCamera.hpp"

#include <Scene/Components/Camera.hpp>

#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>

#include <algorithm>
#include <cmath>
#include <numbers>

namespace adh {
    static constexpr int noButton{ -1 };
    static constexpr float lookSpeed{ 0.004f };
    static constexpr float fastFactor{ 4.0f };
    static constexpr float wheelFactor{ 1.2f };
    static constexpr float minFlySpeed{ 0.1f };
    static constexpr float maxFlySpeed{ 1000.0f };
    static constexpr float minDistance{ 0.1f };
    static constexpr float maxPitch{ std::numbers::pi_v<float> / 2.0f - 0.01f };
    static const Vector3D worldUp{ 0.0f, 1.0f, 0.0f };

    struct View {
        Vector3D forward;
        float distance{};
    };

    static View GetView(const Camera3D& camera) noexcept {
        const Vector3D toFocus{ camera.focusPosition - camera.eyePosition };
        const float distance{ math::magnitude(toFocus) };
        if (!(distance > 0.0f)) {
            return { Vector3D{ 0.0f, 0.0f, 1.0f }, 1.0f };
        }
        return { toFocus / distance, distance };
    }

    static Vector3D Direction(float yaw, float pitch) noexcept {
        return Vector3D{ std::cos(pitch) * std::sin(yaw), std::sin(pitch), std::cos(pitch) * std::cos(yaw) };
    }

    static float HalfFieldOfView(const Camera3D& camera) noexcept {
        return math::to_radians(std::clamp(camera.fieldOfView, 1.0f, 179.0f)) / 2.0f;
    }

    static float NearestFocusDistance(const Camera3D& camera) noexcept {
        return std::max(camera.nearZ, minDistance);
    }

    void EditorCamera::Update(Camera3D& camera, bool hovered, float viewHeight) noexcept {
        const ImGuiIO& io{ ImGui::GetIO() };
        const ImGuiID dragId{ ImGui::GetID("##EditorCamera") };

        bool dragStarted{ false };
        if (m_DragButton == noButton && hovered) {
            for (const ImGuiMouseButton button : { ImGuiMouseButton_Right, ImGuiMouseButton_Middle, ImGuiMouseButton_Left }) {
                if (ImGui::IsMouseClicked(button) && (button != ImGuiMouseButton_Left || io.KeyAlt)) {
                    m_DragButton = button;
                    dragStarted  = true;
                    ImGui::FocusWindow(ImGui::GetCurrentWindow());
                    ImGui::SetActiveID(dragId, ImGui::GetCurrentWindow());
                    break;
                }
            }
        }
        if (m_DragButton != noButton) {
            ImGui::KeepAliveID(dragId);
            if (!ImGui::IsMouseDown(m_DragButton)) {
                m_DragButton = noButton;
            }
        }

        auto [forward, distance]{ GetView(camera) };
        float yaw{ std::atan2(forward.x, forward.z) };
        float pitch{ std::asin(std::clamp(forward.y, -1.0f, 1.0f)) };
        const ImVec2 mouse{ io.MouseDelta };
        const bool mouseMoved{ !dragStarted && (mouse.x != 0.0f || mouse.y != 0.0f) };

        if ((m_DragButton == ImGuiMouseButton_Right || m_DragButton == ImGuiMouseButton_Left) && mouseMoved) {
            yaw += mouse.x * lookSpeed;
            pitch   = std::clamp(pitch - mouse.y * lookSpeed, -maxPitch, maxPitch);
            forward = Direction(yaw, pitch);
            if (m_DragButton == ImGuiMouseButton_Right) {
                camera.focusPosition = camera.eyePosition + forward * distance;
            } else {
                camera.eyePosition = camera.focusPosition - forward * distance;
            }
        }

        const Vector3D right{ std::cos(yaw), 0.0f, -std::sin(yaw) };
        const Vector3D up{ math::cross(forward, right) };

        if (m_DragButton == ImGuiMouseButton_Middle && mouseMoved && viewHeight > 0.0f) {
            const float unitsPerPoint{ 2.0f * distance * std::tan(HalfFieldOfView(camera)) / viewHeight };
            const Vector3D move{ (up * mouse.y - right * mouse.x) * unitsPerPoint };
            camera.eyePosition += move;
            camera.focusPosition += move;
        }

        if (m_DragButton == ImGuiMouseButton_Right) {
            auto axis = [](ImGuiKey positive, ImGuiKey negative) {
                return (ImGui::IsKeyDown(positive) ? 1.0f : 0.0f) - (ImGui::IsKeyDown(negative) ? 1.0f : 0.0f);
            };
            const Vector3D direction{ forward * axis(ImGuiKey_W, ImGuiKey_S) + right * axis(ImGuiKey_D, ImGuiKey_A) +
                                      worldUp * axis(ImGuiKey_E, ImGuiKey_Q) };
            const Vector3D move{ direction * (m_FlySpeed * (io.KeyShift ? fastFactor : 1.0f) * io.DeltaTime) };
            camera.eyePosition += move;
            camera.focusPosition += move;
        }

        if (io.MouseWheel != 0.0f) {
            const float steps{ std::pow(wheelFactor, io.MouseWheel) };
            if (m_DragButton == ImGuiMouseButton_Right) {
                m_FlySpeed = std::clamp(m_FlySpeed * steps, minFlySpeed, maxFlySpeed);
            } else if (m_DragButton == noButton && hovered) {
                camera.eyePosition = camera.focusPosition - forward * std::max(distance / steps, NearestFocusDistance(camera));
            }
        }
    }

    void EditorCamera::Frame(Camera3D& camera, const Vector3D& center, float radius) noexcept {
        const float halfAngle{ std::atan(std::tan(HalfFieldOfView(camera)) * std::clamp(camera.aspectRatio, 0.1f, 1.0f)) };
        const float fit{ radius / std::sin(halfAngle) };
        const float nearest{ NearestFocusDistance(camera) + radius };
        const float farthest{ camera.farZ - radius };
        const float distance{ nearest <= farthest ? std::clamp(fit, nearest, farthest) : (camera.nearZ + camera.farZ) / 2.0f };

        const Vector3D forward{ GetView(camera).forward };
        camera.focusPosition = center;
        camera.eyePosition   = center - forward * distance;
    }

    bool EditorCamera::IsNavigating() const noexcept {
        return m_DragButton != noButton;
    }
} // namespace adh
