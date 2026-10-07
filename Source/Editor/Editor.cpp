#include "Editor.hpp"
#include "EditorActions.hpp"
#include "Panels/MainMenuBar.hpp"
#include "Panels/SaveChangesDialog.hpp"
#include "Panels/Toolbar.hpp"
#include "Shell.hpp"
#include "Theme.hpp"

#include <Scene/Scene.hpp>
#include <Utf8.hpp>
#include <Vulkan/Context.hpp>
#include <Window.hpp>

#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>
#include <ImGuizmo.h>

#include <algorithm>
#include <filesystem>
#include <string>
#include <utility>

namespace adh {
    Editor::~Editor() {
        EventBus().destroy(m_EventSubscriber);
        if (m_ImGuiContext) {
            ImGui::DestroyContext(m_ImGuiContext);
        }
    }

    void Editor::Create(const vk::RenderPass& uiRenderPass, Window& window, vk::Swapchain& swapchain, Scene& scene, vk::Sampler& sampler,
                        VkImageView sceneViewport, VkImageView gameViewport, float aspectRatioWidth, float aspectRatioHeight) {
        m_ImGuiContext = ImGui::CreateContext();
        SetUpConfigFlags();
        theme::ApplyStyle();
        theme::LoadFonts(std::filesystem::path{ vk::Context::Get()->GetDataDirectory() } / "Resources/Fonts");
        SetUpEventCallbacks();

        m_ImGui.Create(uiRenderPass, vk::Context::Get()->GetQueue(vk::DeviceQueues::Family::eGraphics).queue, swapchain.GetImageViewCount());

        auto& io{ ImGui::GetIO() };
        io.BackendPlatformName = "AdHoc native window";
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        auto& platform{ ImGui::GetPlatformIO() };
        platform.Platform_ClipboardUserData  = this;
        platform.Platform_GetClipboardTextFn = [](ImGuiContext* context) -> const char* {
            auto& editor{ *static_cast<Editor*>(context->PlatformIO.Platform_ClipboardUserData) };
            editor.m_ClipboardText = editor.m_Context.window->GetClipboardText();
            return editor.m_ClipboardText.c_str();
        };
        platform.Platform_SetClipboardTextFn = [](ImGuiContext* context, const char* text) {
            static_cast<Editor*>(context->PlatformIO.Platform_ClipboardUserData)->m_Context.window->SetClipboardText(text);
        };
        m_Input.OnFocus(window.IsInFocus());

        m_SettingsPath = vk::Context::Get()->GetDataDirectory() + "Editor.ini";
        io.IniFilename = m_SettingsPath.data();

        m_Context.window                  = &window;
        m_Context.scene                   = &scene;
        m_Context.gameAspect.width        = aspectRatioWidth;
        m_Context.gameAspect.height       = aspectRatioHeight;
        m_Context.gameAspect.screenWidth  = aspectRatioWidth;
        m_Context.gameAspect.screenHeight = aspectRatioHeight;

        const std::filesystem::path dataDirectory{ vk::Context::Get()->GetDataDirectory() };
        m_Context.paths = EditorPaths{
            .assets         = dataDirectory / "Assets",
            .scenes         = dataDirectory / "Assets/Scenes",
            .models         = dataDirectory / "Assets/Models",
            .scripts        = dataDirectory / "Assets/Scripts",
            .textures       = dataDirectory / "Assets/Textures",
            .scriptTemplate = dataDirectory / "Resources/Scripts/template.lua",
        };

        m_ScenePanel.texture = m_ImGui.AddTexture("Scene Viewport", sceneViewport, sampler);
        m_GamePanel.texture  = m_ImGui.AddTexture("Game Viewport", gameViewport, sampler);

        m_Context.history.Reset(scene.SaveToText());
    }

    void Editor::UpdateViewportImages(VkImageView sceneViewport, VkImageView gameViewport, vk::Sampler& sampler) {
        m_ImGui.UpdateTexture("Scene Viewport", sceneViewport, sampler);
        m_ImGui.UpdateTexture("Game Viewport", gameViewport, sampler);
    }

    void Editor::OnUpdate(Scene* scene, float deltaTime, bool drawEditor) {
        m_Context.scene = scene;
        m_DeltaTime     = deltaTime;
        if (drawEditor && !m_DrawEditor) {
            m_Input.Reset();
            m_Input.OnFocus(m_Context.window->IsInFocus());
        }
        m_DrawEditor = drawEditor;
        m_Keyboard.OnUpdate();
    }

    bool Editor::GetKeyDown(std::uint64_t keycode) noexcept {
        return m_Keyboard.GetKeyState(keycode) == KeyboardEvent::Type::eKeyDown;
    }

    float Editor::GetSelectedAspectRatio() const noexcept {
        return m_Context.gameAspect.GetRatio();
    }

    float Editor::GetSelectedAspectRatioWidth() const noexcept {
        return m_Context.gameAspect.width;
    }

    float Editor::GetSelectedAspectRatioHeight() const noexcept {
        return m_Context.gameAspect.height;
    }

    const ViewportImage& Editor::GetSceneViewportImage() const noexcept {
        return m_ScenePanel.image;
    }

    const ViewportImage& Editor::GetGameViewportImage() const noexcept {
        return m_GamePanel.image;
    }

    const std::optional<ScenePick>& Editor::GetScenePick() const noexcept {
        return m_ScenePanel.pick;
    }

    void Editor::SelectEntity(ecs::Entity entity) {
        actions::SelectEntity(m_Context, entity);
    }

    ecs::Entity Editor::GetSelectedEntity() const noexcept {
        return m_Context.selectedEntity;
    }

    bool Editor::IsAskingToSave() const noexcept {
        return m_IsCloseRequested || m_Context.actionAfterSaveQuestion;
    }

    void Editor::Draw(VkCommandBuffer cmd, std::uint32_t imageIndex) {
        m_ImGui.Draw(cmd, imageIndex);
    }

    void Editor::BeginDockSpace() noexcept {
        constexpr ImGuiDockNodeFlags dockspaceFlags{ ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton };

        ImGuiWindowFlags window_flags{ ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking };

        const ImGuiViewport* viewport{ ImGui::GetMainViewport() };
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ ImGui::GetStyle().FramePadding.x, ImGui::GetStyle().FramePadding.y + 1.0f });
        ImGui::Begin("Dockspace", nullptr, window_flags);
        ImGui::PopStyleVar(4);

        ImGuiID dockspaceId{ ImGui::GetID("MyDockSpace") };
        if (m_ResetLayout || !ImGui::DockBuilderGetNode(dockspaceId)) {
            BuildDefaultLayout(dockspaceId);
            m_ResetLayout = false;
        }

        ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockspaceFlags);
    }

    void Editor::BuildDefaultLayout(ImGuiID dockspaceId) noexcept {
        ImGui::DockBuilderRemoveNode(dockspaceId);
        ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetContentRegionAvail());

        ImGuiID center{ dockspaceId };
        const ImGuiID right{ ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.3f, nullptr, &center) };
        const ImGuiID bottom{ ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.25f, nullptr, &center) };
        const ImGuiID left{ ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.26f, nullptr, &center) };

        ImGui::DockBuilderDockWindow(m_SceneHierarchyPanel.GetTitle(), left);
        ImGui::DockBuilderDockWindow(m_ScenePanel.GetTitle(), center);
        ImGui::DockBuilderDockWindow(m_GamePanel.GetTitle(), center);
        ImGui::DockBuilderDockWindow(m_InspectorPanel.GetTitle(), right);
        ImGui::DockBuilderDockWindow(m_SettingsPanel.GetTitle(), right);
        ImGui::DockBuilderDockWindow(m_AssetPanel.GetTitle(), bottom);
        ImGui::DockBuilderDockWindow(m_ConsolePanel.GetTitle(), bottom);
        ImGui::DockBuilderFinish(dockspaceId);

        m_ShowsPlaying.reset();
    }

    void Editor::EndDockSpace() noexcept {
        ImGui::End();
    }

    void Editor::NewFrame(bool* maximizeOnPlay, bool* play, bool* pause, bool* fpsLimit, RenderSettings& renderSettings) {
        m_Context.renderSettings = &renderSettings;
        m_Context.fpsLimit       = fpsLimit;
        m_Context.maximizeOnPlay = maximizeOnPlay;
        m_Context.isPlaying      = *play;
        m_Context.isPaused       = *pause;

        shell::PollFileDialog();

        ImGuiIO& io{ ImGui::GetIO() };
        io.DisplaySize             = { static_cast<float>(m_Context.window->GetLogicalWidth()), static_cast<float>(m_Context.window->GetLogicalHeight()) };
        io.DisplayFramebufferScale = {
            io.DisplaySize.x > 0.0f ? static_cast<float>(m_Context.window->GetWindowWidth()) / io.DisplaySize.x : 1.0f,
            io.DisplaySize.y > 0.0f ? static_cast<float>(m_Context.window->GetWindowHeight()) / io.DisplaySize.y : 1.0f
        };
        io.DeltaTime = std::max(m_DeltaTime, 1.0e-6f);
        constexpr Window::Cursor cursors[ImGuiMouseCursor_COUNT]{ Window::Cursor::eArrow, Window::Cursor::eTextInput, Window::Cursor::eResizeAll,
                                                                  Window::Cursor::eResizeNS, Window::Cursor::eResizeEW, Window::Cursor::eResizeNESW, Window::Cursor::eResizeNWSE,
                                                                  Window::Cursor::eHand, Window::Cursor::eArrow, Window::Cursor::eArrow, Window::Cursor::eNotAllowed };
        const ImGuiMouseCursor cursor{ ImGui::GetMouseCursor() };
        m_Context.window->SetCursor(io.MouseDrawCursor || cursor == ImGuiMouseCursor_None ? Window::Cursor::eHidden : cursors[cursor]);
        EditorInput::NewFrame();
        ImGuizmo::BeginFrame();
        BeginDockSpace();

        if (m_Context.renamedEntity != m_Context.selectedEntity) {
            m_Context.renamedEntity = ecs::NULL_ENTITY;
        }

        if (m_IsCloseRequested && !EditorInput::HasQueuedEvents()) {
            m_IsCloseRequested = false;
            actions::CloseWindow(m_Context);
        }

        const MainMenuBarResult menuBar{ DrawMainMenuBar(m_Context, m_Panels) };
        if (menuBar.resetLayout) {
            m_ResetLayout = true;
        }
        DrawToolbar(m_Context);

        const bool barHasKeyboard{ menuBar.hasKeyboard || ImGui::IsWindowFocused() };
        if (barHasKeyboard && m_KeyboardPanel && m_KeyboardPanel->isOpen && !ImGui::IsAnyItemActive()) {
            ImGui::SetWindowFocus(m_KeyboardPanel->GetTitle());
        }

        DrawSaveChangesDialog(m_Context);

        Panel* panelToFocus{};
        const bool isPopupOpen{ ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel) };
        if (m_ShowsPlaying != m_Context.isPlaying && !isPopupOpen) {
            m_ShowsPlaying = m_Context.isPlaying;
            if (m_Context.isPlaying) {
                panelToFocus = &m_GamePanel;
            } else {
                panelToFocus = &m_ScenePanel;
            }
        }

        for (Panel* panel : m_Panels) {
            panel->Draw(m_Context);
            if (panel->isFocused) {
                m_KeyboardPanel = panel;
            }
        }

        if (panelToFocus && panelToFocus->isOpen) {
            ImGui::SetWindowFocus(panelToFocus->GetTitle());
        }

        const bool keysEditScene{ (m_ScenePanel.isFocused || m_SceneHierarchyPanel.isFocused || m_InspectorPanel.isFocused) && !m_ScenePanel.IsMovingCamera() };
        actions::RunShortcuts(m_Context, keysEditScene);

        EndDockSpace();

        // ImGui::ShowDemoWindow();

        shell::DrawFileDialog();
        actions::EndEditFrame(m_Context);
        ImGui::Render();

        actions::ApplyDeferred(m_Context);

        const std::string name{ m_Context.scenePath.empty() ? std::string{ "Untitled" } : ToUtf8(m_Context.scenePath.filename()) };
        const std::string title{ name + (actions::HasUnsavedChanges(m_Context) ? "*" : "") + " - AdHoc" };
        if (title != m_WindowTitle) {
            m_Context.window->SetTitle(title.c_str());
            m_WindowTitle = title;
        }
    }

    void Editor::SetUpConfigFlags() const noexcept {
        auto& io{ ImGui::GetIO() };
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#if defined(ADH_APPLE)
        io.ConfigMacOSXBehaviors = true;
#endif

        io.BackendRendererName = "ImGuiVulkan";
        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
    }

    void Editor::SetUpEventCallbacks() {
        m_EventSubscriber = EventBus().create_subscriber();
        EventBus().subscribe<MouseMoveEvent>(m_EventSubscriber, this, &Editor::OnMouseMoveEvent);
        EventBus().subscribe<MouseButtonEvent>(m_EventSubscriber, this, &Editor::OnMouseButtonEvent);
        EventBus().subscribe<MouseWheelEvent>(m_EventSubscriber, this, &Editor::OnMouseWheelEvent);
        EventBus().subscribe<KeyboardEvent>(m_EventSubscriber, this, &Editor::OnKeyboardEvent);
        EventBus().subscribe<CharEvent>(m_EventSubscriber, this, &Editor::OnCharEvent);
        EventBus().subscribe<WindowEvent>(m_EventSubscriber, this, &Editor::OnWindowEvent);
    }

    bool Editor::OnKeyboardEvent(KeyboardEvent& event) noexcept {
        m_Keyboard.SetKeyState(&event);

        if (m_DrawEditor) {
            m_Input.OnKeyboard(event);
            return ImGui::GetIO().WantCaptureKeyboard && !m_ScenePanel.isFocused && !m_GamePanel.isFocused;
        }
        return false;
    }

    bool Editor::OnCharEvent(CharEvent& event) noexcept {
        if (m_DrawEditor) {
            m_Input.OnCharacter(event);
            return ImGui::GetIO().WantTextInput;
        }
        return false;
    }

    bool Editor::OnMouseMoveEvent(MouseMoveEvent& event) noexcept {
        if (m_DrawEditor) {
            m_Input.OnMouseMove(event);
            bool handled{ ImGui::GetIO().WantCaptureMouse &&
                          !m_ScenePanel.rect.IsInViewportRect(event.x, event.y) &&
                          !m_GamePanel.rect.IsInViewportRect(event.x, event.y) };

            event.x = static_cast<std::int16_t>(event.x - m_ScenePanel.rect.left);
            event.y = static_cast<std::int16_t>(event.y - m_ScenePanel.rect.top);
            return handled;
        }
        return false;
    }

    bool Editor::OnMouseButtonEvent(MouseButtonEvent& event) noexcept {
        if (m_DrawEditor) {
            m_Input.OnMouseButton(event);
            return ImGui::GetIO().WantCaptureMouse &&
                   !m_ScenePanel.rect.IsInViewportRect(event.x, event.y) &&
                   !m_GamePanel.rect.IsInViewportRect(event.x, event.y);
        }
        return false;
    }

    bool Editor::OnMouseWheelEvent(MouseWheelEvent& event) noexcept {
        if (m_DrawEditor) {
            m_Input.OnMouseWheel(event);
            return ImGui::GetIO().WantCaptureMouse &&
                   !m_ScenePanel.rect.IsInViewportRect(event.x, event.y) &&
                   !m_GamePanel.rect.IsInViewportRect(event.x, event.y);
        }
        return false;
    }

    bool Editor::OnWindowEvent(WindowEvent& event) noexcept {
        if (event.type == WindowEvent::Type::eKillfocus) {
            m_Keyboard.OnKillFocus();
            m_Input.OnFocus(false);
        } else if (event.type == WindowEvent::Type::eFocus) {
            m_Input.OnFocus(true);
        } else if (event.type == WindowEvent::Type::eCloseRequested) {
            m_IsCloseRequested = true;
            m_Context.window->Restore();
        }
        return false;
    }
} // namespace adh
