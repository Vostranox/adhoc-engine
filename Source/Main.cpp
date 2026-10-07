#include <Audio/Audio.hpp>
#include <Editor/Editor.hpp>
#include <Event/Event.hpp>
#include <Input/Input.hpp>
#include <Math/Math.hpp>
#include <Renderer/EntityPicker.hpp>
#include <Renderer/SceneRenderer.hpp>
#include <Scene/Components.hpp>
#include <Scene/ParticleSystem.hpp>
#include <Scene/Scene.hpp>
#include <Utility.hpp>
#include <Vulkan/Attachments.hpp>
#include <Vulkan/CommandBuffer.hpp>
#include <Vulkan/CommandPool.hpp>
#include <Vulkan/Context.hpp>
#include <Vulkan/DescriptorSet.hpp>
#include <Vulkan/Framebuffer.hpp>
#include <Vulkan/ImageView.hpp>
#include <Vulkan/IndexBuffer.hpp>
#include <Vulkan/Memory.hpp>
#include <Vulkan/PipelineLayout.hpp>
#include <Vulkan/RenderPass.hpp>
#include <Vulkan/Sampler.hpp>
#include <Vulkan/Scissor.hpp>
#include <Vulkan/Subpass.hpp>
#include <Vulkan/Swapchain.hpp>
#include <Vulkan/Texture2D.hpp>
#include <Vulkan/Tools.hpp>
#include <Vulkan/UniformBuffer.hpp>
#include <Vulkan/VertexBuffer.hpp>
#include <Vulkan/Viewport.hpp>
#include <Window.hpp>

#include <adh/entity.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <utility>

#if defined(ADH_IOS)
#    include <UIKit/UIKit.h>
#endif

using namespace adh;
using namespace adh::vk;

#include <Scripting/Script.hpp>
#include <Scripting/ScriptHandler.hpp>

struct PBR_UBO {
    xmm::Matrix viewProj{ 1.0f };
    xmm::Matrix inverseViewProj{ 1.0f };
};

struct FragmentData {
    alignas(16) Vector3D ambient;
    alignas(16) Vector3D cameraPosition;
};

static VkRect2D LetterboxRect(VkExtent2D extent, float aspectWidth, float aspectHeight) noexcept {
    float width{ static_cast<float>(extent.width) };
    float height{ static_cast<float>(extent.height) };
    float x{};
    float y{};

    if (width * aspectHeight > height * aspectWidth) {
        const float letterboxWidth{ height * aspectWidth / aspectHeight };
        x     = (width - letterboxWidth) / 2.0f;
        width = letterboxWidth;
    } else if (width * aspectHeight < height * aspectWidth) {
        const float letterboxHeight{ width * aspectHeight / aspectWidth };
        y      = (height - letterboxHeight) / 2.0f;
        height = letterboxHeight;
    }

    return { { static_cast<std::int32_t>(x), static_cast<std::int32_t>(y) },
             { static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height) } };
}

struct CollisionPair {
    std::uint64_t e[2];
    CollisionEvent::Type type;
};

class AdHoc {
  public:
    const char* name{ "AdHoc" };
    static constexpr std::int32_t windowWidth{ 1200 };
    static constexpr std::int32_t windowHeight{ 800 };
    adh::Window window;
    Context context;
    Swapchain swapchain;
    std::uint32_t swapchanImageCount{ 2 };

    Sampler sampler;
    Scene scene;
    Editor editor;
    Array<Framebuffer> swapchainFramebuffers;
    Viewport viewport;
    Scissor scissor;
    RenderPass renderPass;
    VkQueue graphicsQueue;
    CommandPool commandPool;
    CommandBuffer commandBuffer;
    Array<VkFence> fence1;
    Array<VkFence> fence2;
    Array<VkSemaphore> presentSempahore;
    Array<VkSemaphore> renderSemaphore;
    Input input;
    UniformBuffer lightBuffer;

    std::uint32_t currentFrame{};
    std::uint32_t imageIndex{};
    bool renderingReady = false;
    DirectionalLight directionalLight;

    struct CameraView {
        SceneCamera camera;
        PBR_UBO viewProjection;
        UniformBuffer viewProjectionBuffer;
        FragmentData fragmentData;
        UniformBuffer fragmentDataBuffer;
        SunCascades cascades;
        UniformBuffer cascadesBuffer;
        DescriptorSet descriptorSet;
        SceneView target;
        bool isActive{};
    };

    CameraView sceneView;
    CameraView gameView;

    bool g_DrawEditor{ true };
    bool g_EditorFpsLimit{ true };
    bool g_MaximizeOnPlay{ false };
    bool maximizeOnPlay2{ false };

    bool g_IsPlaying{ false };
    bool g_IsPaused{ false };
    bool g_AreScriptsReady{ false };
    bool hasPhysicsPoses{ false };

    std::vector<std::function<void()>> collisionCallbacks;
    std::vector<CollisionPair> collisionCallbacks2;

    SceneRenderer sceneRenderer;
    std::vector<MeshDraw> meshDraws;
    ParticleSystem particleSystem;
    EntityPicker entityPicker;

    AudioDevice audioDevice;

    event::Subscriber eventSubscriber{ event::NULL_SUBSCRIBER };

  public:
    ~AdHoc() {
        EventBus().destroy(eventSubscriber);
        auto device{ Context::Get()->GetDevice() };
        vkDeviceWaitIdle(device);
        for (std::uint32_t i{}; i != swapchain.GetImageViewCount(); ++i) {
            vkDestroySemaphore(device, presentSempahore[i], nullptr);
            vkDestroySemaphore(device, renderSemaphore[i], nullptr);
            vkDestroyFence(device, fence1[i], nullptr);
        }
        Mesh::Clear();
        MaterialTextures::DestroyDefaults();
        Texture2D::CleanUpDefaultSamplers();
        TextureDescriptors::CleanUp();
    }

    bool OnCollisionEvent(CollisionEvent& event) noexcept {
        if (g_IsPlaying && g_AreScriptsReady) {
            CollisionPair p;
            p.e[0] = event.entityA;
            p.e[1] = event.entityB;
            p.type = event.type;

            collisionCallbacks2.emplace_back(p);
        }
        return true;
    }

    bool OnStatusEvent(StatusEvent& event) noexcept {
        switch (event.type) {
        case StatusEvent::Type::eRun:
            {
                scene.Save();
                scene.ResetPhysicsWorld();
                particleSystem.Reset();
                ReadyScript();
                g_AreScriptsReady = true;
                g_IsPlaying       = true;
                g_IsPaused        = false;
                hasPhysicsPoses   = false;

                maximizeOnPlay2 = g_MaximizeOnPlay;
                collisionCallbacks.clear();
                break;
            }
        case StatusEvent::Type::eStop:
            {
                if (g_IsPlaying) {
                    Mesh::Clear();
                    scene.Load();
                    g_AreScriptsReady = false;
                    g_IsPlaying       = false;
                    g_IsPaused        = false;
                    g_MaximizeOnPlay  = maximizeOnPlay2;
                    collisionCallbacks.clear();
                }
                break;
            }
        case StatusEvent::Type::ePause:
            {
                g_IsPaused = true;
                break;
            }
        case StatusEvent::Type::eUnpause:
            {
                g_IsPaused = false;
                break;
            }
        }
        return true;
    }

    void Initialize(const char* path) {
        window.Create(name, windowWidth, windowHeight, true, false);
        context.Create(window, "AdHoc", path);
        swapchain.Create(swapchanImageCount, VK_FORMAT_B8G8R8A8_UNORM, VK_PRESENT_MODE_FIFO_KHR);
        InitializeRenderPass();

        sampler.Create(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_COMPARE_OP_NEVER, VK_FALSE, VK_TRUE);

        TextureDescriptors::Initialize();
        Texture2D::InitializeDefaultSamplers();
        MaterialTextures::CreateDefaults();

        sceneRenderer.Create(renderPass, sampler, swapchain.GetImageViewCount());
        InitializeDescriptorSets();
        commandBuffer.Create(VK_COMMAND_BUFFER_LEVEL_PRIMARY, swapchain.GetImageViewCount(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, DeviceQueues::Family::eGraphics);
        InitializeFramebuffers();
        InitializeSyncData();

        InitializeScripting();

        sceneRenderer.Resize(sceneView.target, { SceneRenderer::minViewSize, SceneRenderer::minViewSize });
        sceneRenderer.Resize(gameView.target, { SceneRenderer::minViewSize, SceneRenderer::minViewSize });
        entityPicker.Create();
        scene.ResetToDefault();
        CreateEditor();

        eventSubscriber = EventBus().create_subscriber();
        EventBus().subscribe<WindowEvent>(eventSubscriber, this, &AdHoc::OnResize);
        EventBus().subscribe<StatusEvent>(eventSubscriber, this, &AdHoc::OnStatusEvent);
        EventBus().subscribe<CollisionEvent>(eventSubscriber, this, &AdHoc::OnCollisionEvent);
        input.Initialize();

        audioDevice.Create();

        renderingReady = true;
    }

    void Run() {
        auto start            = std::chrono::steady_clock::now();
        const float maxFPS    = 60.0f;
        const float maxPeriod = 1.0f / maxFPS;

        while (window.IsOpen()) {
            if (!renderingReady) {
                continue;
            }

            while (window.IsOpen() && window.IsMinimized()) {
                window.WaitEvents();
            }

            auto end                 = std::chrono::steady_clock::now();
            float deltaTime          = std::chrono::duration<float>(end - start).count();
            ScriptHandler::deltaTime = deltaTime;

            if (!g_EditorFpsLimit || g_IsPlaying || deltaTime >= maxPeriod) {
                editor.OnUpdate(&scene, deltaTime, g_DrawEditor);
                input.OnUpdate();
                input.PollEvents();
                window.PollEvents();

                UpdateGameCamera();
                UpdateScripts(deltaTime);

                if (g_IsPlaying && !g_IsPaused) {
                    scene.GetPhysics().StepSimulation(deltaTime);
                    scene.GetWorld().get_system<Transform, RigidBody>().for_each([&](Transform& transform, RigidBody& rigidBody) {
                        rigidBody.OnUpdate(transform);
                    });
                    hasPhysicsPoses = true;
                    particleSystem.Update(scene.GetWorld(), deltaTime);
                }

                if (g_IsPlaying && g_MaximizeOnPlay) {
                    g_DrawEditor = false;
                } else {
                    g_DrawEditor = true;
                }

                if (g_IsPlaying && (editor.GetKeyDown(ADH_ESCAPE) || editor.IsAskingToSave())) {
                    g_MaximizeOnPlay = false;
                }

                if (swapchain.isValid && !window.IsMinimized()) {
                    Draw();
                } else if (!swapchain.isValid && !window.IsMinimized() && !IsSurfaceZeroSized()) {
                    RecreateSwapchain();
                }

                if (ScriptHandler::loadSceneFilename) {
                    const auto filename = std::exchange(ScriptHandler::loadSceneFilename, std::nullopt);
                    if (scene.LoadFromFile(Context::Get()->GetDataDirectory() + "Assets/Scenes/" + *filename)) {
                        editor.SelectEntity(ecs::NULL_ENTITY);
                        scene.ResetPhysicsWorld();
                        ReadyScript();
                    }
                }
                while (!ScriptHandler::scriptComponentEvent.IsEmpty()) {
                    auto events = std::exchange(ScriptHandler::scriptComponentEvent, {});
                    for (auto& event : events) {
                        event();
                    }
                }
                start = end;
            }
        }
    }

    static void SetViewCamera(CameraView& view, auto& camera) {
        const xmm::Matrix cameraView{ camera.GetXmmView() };
        view.camera.viewProjection          = camera.GetXmmProjection() * cameraView;
        view.viewProjection.viewProj        = view.camera.viewProjection;
        view.viewProjection.inverseViewProj = xmm::inverse(view.camera.viewProjection);
        view.fragmentData.cameraPosition    = camera.eyePosition;
        view.camera.view                    = cameraView;
        view.camera.projection              = camera.GetXmmProjection();
        view.camera.right                   = Vector3D{ cameraView.f[0], cameraView.f[4], cameraView.f[8] };
        view.camera.up                      = Vector3D{ cameraView.f[1], cameraView.f[5], cameraView.f[9] };
        view.camera.skyViewProjection       = camera.GetXmmProjection() * XmmLookAt(Vector3D{}, camera.focusPosition - camera.eyePosition, camera.upVector);
    }

    void UpdateGameCamera() {
        scene.GetWorld().get_system<Camera2D>().for_each([&](Camera2D& camera) {
            auto width    = editor.GetSelectedAspectRatioWidth();
            auto height   = editor.GetSelectedAspectRatioHeight();
            camera.left   = -(width / 2.0f);
            camera.right  = width / 2.0f;
            camera.bottom = -(height / 2.0f);
            camera.top    = height / 2.0f;

            if (camera.isRuntimeCamera) {
                SetViewCamera(gameView, camera);
            }
        });
        scene.GetWorld().get_system<Camera3D>().for_each([&](Camera3D& camera) {
            if (camera.isRuntimeCamera) {
                camera.aspectRatio = editor.GetSelectedAspectRatio();
                SetViewCamera(gameView, camera);
            }
        });
    }

    void UpdateSceneCamera() {
        scene.GetWorld().get_system<Camera2D>().for_each([&](Camera2D& camera) {
            if (camera.isSceneCamera) {
                SetViewCamera(sceneView, camera);
            }
        });
        scene.GetWorld().get_system<Camera3D>().for_each([&](Camera3D& camera) {
            if (camera.isSceneCamera) {
                SetViewCamera(sceneView, camera);
            }
        });
    }

    std::vector<ecs::Entity> ScriptedEntities() {
        std::vector<ecs::Entity> entities;
        scene.GetWorld().get_system<Script>().for_each([&](ecs::Entity ent, Script&) {
            entities.push_back(ent);
        });
        return entities;
    }

    void ReadyScript() {
        auto& world{ scene.GetWorld() };
        const auto entities{ ScriptedEntities() };
        for (const auto ent : entities) {
            if (world.has_component<Script>(ent)) {
                auto& script{ world.get<Script>(ent) };
                script.Compile(scene.GetState());
                script::Script instance{ script.instance };
                ScriptHandler::currentEntity = static_cast<std::uint64_t>(ent);
                instance.run();
            }
        }

        for (const auto ent : entities) {
            if (world.has_component<Script>(ent)) {
                script::Script instance{ world.get<Script>(ent).instance };
                ScriptHandler::currentEntity = static_cast<std::uint64_t>(ent);
                instance.call("Start");
            }
        }
    }

    void UpdateScripts(float deltaTime) {
        if (g_IsPlaying && !g_IsPaused && g_AreScriptsReady) {
            auto& world{ scene.GetWorld() };
            for (const auto ent : ScriptedEntities()) {
                if (!world.has_component<Script>(ent)) {
                    continue;
                }
                script::Script instance{ world.get<Script>(ent).instance };
                instance.bind();
                ScriptHandler::currentEntity = static_cast<std::uint64_t>(ent);
                instance.call_bound("Update");
                if (world.has_component<Script>(ent)) {
                    auto& script{ world.get<Script>(ent) };
                    script.fixedUpdateAccumulator += deltaTime;
                    if (script.fixedUpdateAccumulator >= 0.017f) {
                        script.fixedUpdateAccumulator = {};
                        instance.call_bound("FixedUpdate");
                    }
                }

                auto ee = static_cast<std::uint64_t>(ent);
                for (std::size_t i{}; i != collisionCallbacks2.size(); ++i) {
                    bool call = false;
                    std::uint64_t rhs;
                    if (collisionCallbacks2[i].e[0] == ee) {
                        rhs  = collisionCallbacks2[i].e[1];
                        call = true;
                    } else if (collisionCallbacks2[i].e[1] == ee) {
                        rhs  = collisionCallbacks2[i].e[0];
                        call = true;
                    }

                    if (call && world.has_component<Script>(ent)) {
                        switch (collisionCallbacks2[i].type) {
                        case CollisionEvent::Type::eCollisionInvalid:
                            {
                                break;
                            }
                        case CollisionEvent::Type::eCollisionEnter:
                            {
                                instance.call_bound("OnCollisionEnter", rhs);
                                break;
                            }
                        case CollisionEvent::Type::eCollisionPersist:
                            {
                                instance.call_bound("OnCollisionPersist", rhs);
                                break;
                            }
                        case CollisionEvent::Type::eCollisionExit:
                            {
                                instance.call_bound("OnCollisionExit", rhs);
                                break;
                            }
                        case CollisionEvent::Type::eTriggerEnter:
                            {
                                instance.call_bound("OnTriggerEnter", rhs);
                                break;
                            }
                        case CollisionEvent::Type::eTriggerPersist:
                            {
                                instance.call_bound("OnTriggerPersist", rhs);
                                break;
                            }
                        case CollisionEvent::Type::eTriggerExit:
                            {
                                instance.call_bound("OnTriggerExit", rhs);
                                break;
                            }
                        }
                    }
                }

                instance.unbind();
            }
            collisionCallbacks2.clear();
        }
    }

    xmm::Matrix ModelMatrix(ecs::Entity e, Transform& transform) {
        if (g_IsPlaying && hasPhysicsPoses && scene.GetWorld().has_component<RigidBody>(e)) {
            return transform.GetXmmPhysics();
        }
        return transform.GetXmm();
    }

    void GatherMeshes() {
        meshDraws.clear();
        scene.GetWorld().get_system<Transform, Mesh, Material>().for_each([&](ecs::Entity e, Transform& transform, Mesh& mesh, Material& material) {
            if (mesh.toDraw && mesh.GetIndexCount() > 0) {
                const MaterialTextures* textures{ scene.GetWorld().has_component<MaterialTextures>(e) ? &scene.GetWorld().get<MaterialTextures>(e) : nullptr };
                const VkDescriptorSet materialSet{ textures ? textures->GetDescriptorSet() : MaterialTextures::GetDefaultDescriptorSet() };
                const bool blended{ material.transparency < 1.0f || (textures && !textures->IsOpaque()) };
                meshDraws.push_back({ &mesh, ModelMatrix(e, transform), material, materialSet, e, blended });
            }
        });
        std::ranges::stable_partition(meshDraws, [](const MeshDraw& draw) { return !draw.blended; });
    }

    static VkExtent2D ViewExtent(float width, float height) noexcept {
        const auto pixelWidth{ static_cast<std::uint32_t>(std::lround(std::max(width, 0.0f))) };
        const auto pixelHeight{ static_cast<std::uint32_t>(std::lround(std::max(height, 0.0f))) };
        return { std::max(pixelWidth, SceneRenderer::minViewSize), std::max(pixelHeight, SceneRenderer::minViewSize) };
    }

    static bool NeedsResize(const CameraView& view, VkExtent2D extent) noexcept {
        return view.isActive && (extent.width != view.target.extent.width || extent.height != view.target.extent.height);
    }

    void PrepareViews(VkRect2D letterbox) {
        const ViewportImage& scenePanel{ editor.GetSceneViewportImage() };
        const ViewportImage& gamePanel{ editor.GetGameViewportImage() };
        sceneView.isActive = g_DrawEditor && scenePanel.isVisible;
        gameView.isActive  = !g_DrawEditor || gamePanel.isVisible;
        const VkExtent2D sceneExtent{ ViewExtent(scenePanel.width, scenePanel.height) };
        const VkExtent2D gameExtent{ g_DrawEditor ? ViewExtent(gamePanel.width, gamePanel.height)
                                                  : ViewExtent(static_cast<float>(letterbox.extent.width), static_cast<float>(letterbox.extent.height)) };

        const bool resizeScene{ NeedsResize(sceneView, sceneExtent) };
        const bool resizeGame{ NeedsResize(gameView, gameExtent) };
        if (resizeScene || resizeGame) {
            vkDeviceWaitIdle(Context::Get()->GetDevice());
            if (resizeScene) {
                sceneRenderer.Resize(sceneView.target, sceneExtent);
            }
            if (resizeGame) {
                sceneRenderer.Resize(gameView.target, gameExtent);
            }
            editor.UpdateViewportImages(sceneView.target.ldr.GetImageView(), gameView.target.ldr.GetImageView(), sampler);
        }
    }

    void Draw() {
        auto device               = Context::Get()->GetDevice();
        constexpr auto maxTimeout = std::numeric_limits<std::uint64_t>::max();

        if (g_DrawEditor) {
            editor.NewFrame(&g_MaximizeOnPlay, &g_IsPlaying, &g_IsPaused, &g_EditorFpsLimit, sceneRenderer.settings);
            UpdateSceneCamera();
        }

        const VkRect2D letterbox{ LetterboxRect(swapchain.GetExtent(), editor.GetSelectedAspectRatioWidth(), editor.GetSelectedAspectRatioHeight()) };
        PrepareViews(letterbox);

        sceneRenderer.skybox.Prepare(scene.GetSkybox());

        ADH_THROW(vkWaitForFences(device, 1u, &fence1[currentFrame], VK_TRUE, maxTimeout) == VK_SUCCESS,
                  "Failed to wait for fences!");

        auto acquireNextImage{ vkAcquireNextImageKHR(device, swapchain, maxTimeout, presentSempahore[currentFrame], nullptr, &imageIndex) };
        if (acquireNextImage == VK_ERROR_OUT_OF_DATE_KHR) {
            swapchain.isValid = false;
            return;
        }
        ADH_THROW(acquireNextImage == VK_SUCCESS || acquireNextImage == VK_SUBOPTIMAL_KHR, "Failed to acquire swapchain image!");

        if (fence2[imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(device, 1, &fence2[imageIndex], VK_TRUE, maxTimeout);
        }
        fence2[imageIndex] = fence1[currentFrame];

        auto cmd = commandBuffer.Begin(currentFrame);

        for (CameraView* view : { &sceneView, &gameView }) {
            view->viewProjectionBuffer.Update(imageIndex);
            view->fragmentDataBuffer.Update(imageIndex);
            view->cascades = SunShadows::Fit(view->camera, sceneRenderer.settings);
            view->cascadesBuffer.Update(imageIndex);
        }

        directionalLight.direction = sceneRenderer.settings.sunPosition;
        directionalLight.intensity = sceneRenderer.settings.sunIntensity;
        lightBuffer.Update(imageIndex);

        sceneRenderer.lights.Gather(scene.GetWorld());
        sceneRenderer.lights.Upload(imageIndex);

        sceneRenderer.particles.Upload(scene.GetWorld(), imageIndex);

        GatherMeshes();

        for (CameraView* view : { &sceneView, &gameView }) {
            if (view->isActive) {
                const ecs::Entity outlined{ view == &sceneView ? editor.GetSelectedEntity() : ecs::NULL_ENTITY };
                sceneRenderer.Render(cmd, imageIndex, view->target, view->camera, view->cascades, view->descriptorSet, meshDraws, outlined);
            }
        }

        if (const auto& pick{ editor.GetScenePick() }; pick && sceneView.isActive) {
            entityPicker.Record(cmd, sceneView.target, pick->u, pick->v);
        }

        if (!g_DrawEditor) {
            renderPass.SetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        }
        renderPass.UpdateRenderArea({ {}, swapchain.GetExtent() });

        renderPass.Begin(cmd, swapchainFramebuffers[imageIndex]);

        if (!g_DrawEditor) {
            const float x{ static_cast<float>(letterbox.offset.x) };
            const float y{ static_cast<float>(letterbox.offset.y) };
            const float width{ static_cast<float>(letterbox.extent.width) };
            const float height{ static_cast<float>(letterbox.extent.height) };
            viewport.Update(VkViewport{ x, y + height, width, -height, 0.0f, 1.0f });
            viewport.Set(cmd);

            scissor.Update(letterbox);
            scissor.Set(cmd);

            vkCmdSetDepthBias(cmd, 0, 0.0f, 0);

            sceneRenderer.Present(cmd, imageIndex, gameView.target);
        } else {
            editor.Draw(cmd, imageIndex);
        }

        renderPass.End(cmd);

        commandBuffer.End(currentFrame);

        {
            VkPipelineStageFlags waitStages[]{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
            VkSemaphore waitSemaphores[]{ presentSempahore[currentFrame] };
            VkSemaphore signalSemaphores[]{ renderSemaphore[imageIndex] };
            VkCommandBuffer commandBuffers[]{ cmd };
            VkSubmitInfo submitInfo{};
            submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount   = std::size(commandBuffers);
            submitInfo.pCommandBuffers      = commandBuffers;
            submitInfo.waitSemaphoreCount   = std::size(waitSemaphores);
            submitInfo.pWaitSemaphores      = waitSemaphores;
            submitInfo.pWaitDstStageMask    = waitStages;
            submitInfo.signalSemaphoreCount = std::size(signalSemaphores);
            submitInfo.pSignalSemaphores    = signalSemaphores;

            vkResetFences(device, 1, &fence1[currentFrame]);
            ADH_THROW(vkQueueSubmit(Context::Get()->GetQueue(DeviceQueues::Family::eGraphics).queue, 1u, &submitInfo, fence1[currentFrame]) == VK_SUCCESS,
                      "Failed to submit to queue!");
        }

        {
            VkSwapchainKHR swapchains[]{ swapchain };
            VkSemaphore waitSemaphores[]{ renderSemaphore[imageIndex] };
            VkPresentInfoKHR presentInfo{};
            presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.waitSemaphoreCount = std::size(waitSemaphores);
            presentInfo.pWaitSemaphores    = waitSemaphores;
            presentInfo.swapchainCount     = std::size(swapchains);
            presentInfo.pSwapchains        = swapchains;
            presentInfo.pImageIndices      = &imageIndex;

            auto presentQueue{ vkQueuePresentKHR(Context::Get()->GetQueue(DeviceQueues::Family::ePrensent).queue, &presentInfo) };
            if (presentQueue == VK_ERROR_OUT_OF_DATE_KHR || presentQueue == VK_SUBOPTIMAL_KHR) {
                swapchain.isValid = false;
            }
        }

        if (entityPicker.IsPending()) {
            ADH_THROW(vkWaitForFences(device, 1u, &fence1[currentFrame], VK_TRUE, maxTimeout) == VK_SUCCESS,
                      "Failed to wait for fences!");
            editor.SelectEntity(entityPicker.Read());
        }

        currentFrame = (currentFrame + 1) % swapchain.GetImageViewCount();
        if (currentFrame % swapchain.GetImageViewCount()) {
            Allocator::Flush();
            TextureDescriptors::Flush();
        }
    }

    void
    InitializeRenderPass() {
        Attachment attachment;
        attachment.AddDescription(
            VK_FORMAT_B8G8R8A8_UNORM,
            VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_CLEAR,
            VK_ATTACHMENT_STORE_OP_STORE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            Attachment::Type::eColor);

        Subpass subpass;
        subpass.AddDescription(VK_PIPELINE_BIND_POINT_GRAPHICS, attachment);
        subpass.AddDependencies(
            VK_SUBPASS_EXTERNAL,
            0u,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_NONE_KHR,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

        subpass.AddDependencies(
            0u,
            VK_SUBPASS_EXTERNAL,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_NONE_KHR);

        VkRect2D renderArea{
            { 0u, 0u },
            { (uint32_t)window.GetWindowWidth(), (uint32_t)window.GetWindowHeight() }
        };

        Array<VkClearValue> clearValues;
        clearValues.Resize(1);
        clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

        renderPass.Create(attachment, subpass, renderArea, Move(clearValues));
        renderPass.UpdateRenderArea({ {}, swapchain.GetExtent() });
    }

    void InitializeDescriptorSets() {
        directionalLight.direction = sceneRenderer.settings.sunPosition;
        directionalLight.color     = { 1.0f, 1.0f, 1.0f };
        directionalLight.intensity = sceneRenderer.settings.sunIntensity;

        lightBuffer.Create(&directionalLight, sizeof(directionalLight), swapchain.GetImageViewCount(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

        InitializeViewDescriptorSet(sceneView);
        InitializeViewDescriptorSet(gameView);
    }

    void InitializeViewDescriptorSet(CameraView& view) {
        const PipelineLayout& sceneLayout{ sceneRenderer.GetSceneLayout() };
        view.descriptorSet.Initialize(VK_PIPELINE_BIND_POINT_GRAPHICS, sceneLayout, swapchain.GetImageViewCount());
        view.descriptorSet.AddPool(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4);
        view.descriptorSet.AddPool(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
        view.descriptorSet.AddPool(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
        view.descriptorSet.Create({ sceneLayout.GetSetLayout()[0], sceneLayout.GetSetLayout()[1] });

        view.fragmentData.ambient = { 0.1f, 0.1f, 0.1f };

        view.viewProjectionBuffer.Create(&view.viewProjection, sizeof(view.viewProjection), swapchain.GetImageViewCount(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        view.fragmentDataBuffer.Create(&view.fragmentData, sizeof(view.fragmentData), swapchain.GetImageViewCount(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        view.descriptorSet.Update(view.viewProjectionBuffer, 0u, 0u, 0u, 1u, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        view.cascadesBuffer.Create(&view.cascades.data, sizeof(view.cascades.data), swapchain.GetImageViewCount(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        view.descriptorSet.Update(view.cascadesBuffer, 0u, 1u, 0u, 1u, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        view.descriptorSet.Update(view.fragmentDataBuffer, 1u, 0u, 0u, 1u, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        view.descriptorSet.Update(lightBuffer, 1u, 1u, 0u, 1u, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        view.descriptorSet.Update(sceneRenderer.shadows.GetDescriptor(), 1u, 2u, 0u, 1u, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        view.descriptorSet.Update(sceneRenderer.lights.GetBuffer(), 1u, 3u, 0u, 1u, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    }

    void InitializeFramebuffers() {
        for (std::size_t i{}; i != swapchain.GetImageViewCount(); ++i) {
            VkImageView viewAttachments[]{
                swapchain.GetImageView()[i]
            };
            swapchainFramebuffers.EmplaceBack().Create(renderPass, std::size(viewAttachments), viewAttachments, swapchain.GetExtent(), 1u);
        }
    }

    void InitializeSyncData() {
        auto info{ initializers::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT) };
        fence2.Resize(swapchain.GetImageViewCount());
        for (std::uint32_t i{}; i != swapchain.GetImageViewCount(); ++i) {
            ADH_THROW(vkCreateFence(Context::Get()->GetDevice(), &info, nullptr, &fence1.EmplaceBack()) == VK_SUCCESS,
                      "Failed to create fence!");
        }

        auto info2{ initializers::SemaphoreCreateInfo() };
        for (std::size_t i{}; i != swapchain.GetImageViewCount(); ++i) {
            ADH_THROW(vkCreateSemaphore(Context::Get()->GetDevice(), &info2, nullptr, &presentSempahore.EmplaceBack()) == VK_SUCCESS,
                      "Failed to create semaphore!");
            ADH_THROW(vkCreateSemaphore(Context::Get()->GetDevice(), &info2, nullptr, &renderSemaphore.EmplaceBack()) == VK_SUCCESS,
                      "Failed to create semaphore!");
        }
    }

    void InitializeScripting() {
        ScriptHandler::input = &input;
        ScriptHandler::scene = &scene;
        ScriptHandler::RegisterBindings();
    }

    void CreateEditor() {
        editor.Create(renderPass, window, swapchain, scene, sampler, sceneView.target.ldr.GetImageView(), gameView.target.ldr.GetImageView(),
                      static_cast<float>(windowWidth), static_cast<float>(windowHeight));
    }

    bool OnResize(WindowEvent& event) noexcept {
        if (event.type == WindowEvent::Type::eResized) {
            swapchain.isValid = false;
        }
        return false;
    }

    bool IsSurfaceZeroSized() {
        auto extent{ tools::GetSurfaceCapabilities(Context::Get()->GetPhysicalDevice(), Context::Get()->GetSurface()).currentExtent };
        return extent.width == 0u || extent.height == 0u;
    }

    void RecreateSwapchain() {
        swapchain.Destroy();
        swapchainFramebuffers.Clear();

        swapchain.Create(swapchanImageCount, VK_FORMAT_B8G8R8A8_UNORM, VK_PRESENT_MODE_FIFO_KHR);
        InitializeFramebuffers();

        renderPass.UpdateRenderArea({ {}, swapchain.GetExtent() });

        currentFrame = 0;
        imageIndex   = 0;
    }
};

#if defined(ADH_WINDOWS)
#    include <Windows.h>
#    define ENTRY_POINT INT WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ INT)
#    define EXE_PATH nullptr
#    define ADH_WIN_CONSOLE                                                                                                                                          \
        AllocConsole();                                                                                                                                              \
        FILE* fDummy;                                                                                                                                                \
        freopen_s(&fDummy, "CONOUT$", "w", stdout);                                                                                                                  \
        freopen_s(&fDummy, "CONOUT$", "w", stderr);                                                                                                                  \
        freopen_s(&fDummy, "CONIN$", "r", stdin);                                                                                                                    \
        std::cout.clear();                                                                                                                                           \
        std::clog.clear();                                                                                                                                           \
        std::cerr.clear();                                                                                                                                           \
        std::cin.clear();                                                                                                                                            \
        HANDLE hConOut = CreateFile(L"CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); \
        HANDLE hConIn  = CreateFile(L"CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);  \
        SetStdHandle(STD_OUTPUT_HANDLE, hConOut);                                                                                                                    \
        SetStdHandle(STD_ERROR_HANDLE, hConOut);                                                                                                                     \
        SetStdHandle(STD_INPUT_HANDLE, hConIn);                                                                                                                      \
        std::wcout.clear();                                                                                                                                          \
        std::wclog.clear();                                                                                                                                          \
        std::wcerr.clear();                                                                                                                                          \
        std::wcin.clear();

#else
#    define ENTRY_POINT int main(int, char** argv)
#    define EXE_PATH argv[0]
#endif

ENTRY_POINT {
#if defined(ADH_WINDOWS) && defined(ADH_DEBUG)
    ADH_WIN_CONSOLE;
#endif
    try {
        AdHoc adhoc;
        adhoc.Initialize(EXE_PATH);
        adhoc.Run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        throw;
    }
    return 0;
}
