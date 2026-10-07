#pragma once
#include "Api/Vulkan/VulkanImGui.hpp"
#include "EditorContext.hpp"
#include "EditorInput.hpp"
#include "Panels/AssetPanel.hpp"
#include "Panels/ConsolePanel.hpp"
#include "Panels/GamePanel.hpp"
#include "Panels/InspectorPanel.hpp"
#include "Panels/SceneHierarchyPanel.hpp"
#include "Panels/ScenePanel.hpp"
#include "Panels/SettingsPanel.hpp"

#include <Event/Event.hpp>
#include <Input/Keyboard.hpp>
#include <Renderer/RenderSettings.hpp>
#include <Vulkan/RenderPass.hpp>
#include <Vulkan/Sampler.hpp>
#include <Vulkan/Swapchain.hpp>

#include <ImGui/imgui.h>

#include <array>
#include <cstdint>
#include <optional>
#include <string>

namespace adh {
    class Window;

    class Editor {
      public:
        Editor() noexcept = default;

        Editor(const Editor& rhs) = delete;

        Editor& operator=(const Editor& rhs) = delete;

        Editor(Editor&& rhs) = delete;

        Editor& operator=(Editor&& rhs) = delete;

        ~Editor();

        void Create(const vk::RenderPass& uiRenderPass, Window& window, vk::Swapchain& swapchain, Scene& scene, vk::Sampler& sampler,
                    VkImageView sceneViewport, VkImageView gameViewport, float aspectRatioWidth, float aspectRatioHeight);

        void UpdateViewportImages(VkImageView sceneViewport, VkImageView gameViewport, vk::Sampler& sampler);

        void OnUpdate(Scene* scene, float deltaTime, bool drawEditor);

        void NewFrame(bool* maximizeOnPlay, bool* play, bool* pause, bool* fpsLimit, RenderSettings& renderSettings);

        void Draw(VkCommandBuffer cmd, std::uint32_t imageIndex);

        bool GetKeyDown(std::uint64_t keycode) noexcept;

        float GetSelectedAspectRatio() const noexcept;

        float GetSelectedAspectRatioWidth() const noexcept;

        float GetSelectedAspectRatioHeight() const noexcept;

        const ViewportImage& GetSceneViewportImage() const noexcept;

        const ViewportImage& GetGameViewportImage() const noexcept;

        const std::optional<ScenePick>& GetScenePick() const noexcept;

        void SelectEntity(ecs::Entity entity);

        ecs::Entity GetSelectedEntity() const noexcept;

        bool IsAskingToSave() const noexcept;

      private:
        void BeginDockSpace() noexcept;

        void EndDockSpace() noexcept;

        void BuildDefaultLayout(ImGuiID dockspaceId) noexcept;

        void SetUpConfigFlags() const noexcept;

        void SetUpEventCallbacks();

        bool OnKeyboardEvent(KeyboardEvent& event) noexcept;

        bool OnCharEvent(CharEvent& event) noexcept;

        bool OnMouseMoveEvent(MouseMoveEvent& event) noexcept;

        bool OnMouseButtonEvent(MouseButtonEvent& event) noexcept;

        bool OnMouseWheelEvent(MouseWheelEvent& event) noexcept;

        bool OnWindowEvent(WindowEvent& event) noexcept;

      private:
        ImGuiContext* m_ImGuiContext{};
        vk::VulkanImGui m_ImGui;
        event::Subscriber m_EventSubscriber{ event::NULL_SUBSCRIBER };
        std::string m_SettingsPath;
        std::string m_WindowTitle;
        std::string m_ClipboardText;
        EditorInput m_Input;

        EditorContext m_Context;

        GamePanel m_GamePanel;
        ScenePanel m_ScenePanel;
        AssetPanel m_AssetPanel;
        ConsolePanel m_ConsolePanel;
        InspectorPanel m_InspectorPanel;
        SceneHierarchyPanel m_SceneHierarchyPanel;
        SettingsPanel m_SettingsPanel;
        std::array<Panel*, 7> m_Panels{ &m_SceneHierarchyPanel, &m_ScenePanel, &m_GamePanel, &m_InspectorPanel, &m_SettingsPanel, &m_AssetPanel, &m_ConsolePanel };
        bool m_ResetLayout{ false };
        std::optional<bool> m_ShowsPlaying;
        Panel* m_KeyboardPanel{};

        Keyboard m_Keyboard;
        float m_DeltaTime{};
        bool m_DrawEditor{};
        bool m_IsCloseRequested{};
    };
} // namespace adh
