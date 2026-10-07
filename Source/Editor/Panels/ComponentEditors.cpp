#include "ComponentEditors.hpp"
#include "../EditorActions.hpp"
#include "../EditorContext.hpp"
#include "../IconFontCppHeaders/IconFontAwesome5.hpp"
#include "../Widgets.hpp"

#include <Math/Math.hpp>
#include <Scene/Components.hpp>
#include <Scene/Scene.hpp>
#include <Scripting/Script.hpp>
#include <Utf8.hpp>

#include <ImGui/imgui.h>

#include <cstdint>
#include <filesystem>
#include <string>

namespace adh {
    template <typename T>
    static bool Has(ecs::World& world, ecs::Entity entity) {
        return world.has_component<T>(entity);
    }

    template <typename T, void (*drawFields)(EditorContext&, T&)>
    static void Draw(EditorContext& context, ecs::Entity entity) {
        drawFields(context, context.scene->GetWorld().get<T>(entity));
    }

    template <typename T>
    static void Reset(ecs::World& world, ecs::Entity entity) {
        world.get<T>(entity) = T{};
    }

    template <typename T>
    static void Remove(ecs::World& world, ecs::Entity entity) {
        world.remove<T>(entity);
    }

    template <typename T>
    static void Add(EditorContext& context, ecs::Entity entity) {
        context.scene->GetWorld().add<T>(entity, T{});
    }

    static void AddMesh(EditorContext& context, ecs::Entity entity) {
        context.scene->GetWorld().add<Mesh>(entity, Mesh{}).Load(ToUtf8(context.paths.models / "cube.obj"));
    }

    static const char* WhyNoSkybox(EditorContext& context, ecs::Entity) {
        return context.scene->GetSkybox() ? "The scene has a sky" : nullptr;
    }

    static const char* WhyNoRigidBody(EditorContext& context, ecs::Entity entity) {
        return context.scene->GetWorld().has_component<Transform>(entity) ? nullptr : "Needs a Transform";
    }

    static void DrawTransform(EditorContext&, Transform& component) {
        widgets::Vector3("Position", component.translate);

        Vector3D rotation{ math::to_degrees(component.rotation[0]), math::to_degrees(component.rotation[1]), math::to_degrees(component.rotation[2]) };
        widgets::Vector3("Rotation", rotation, 0.0f, 0.5f);
        component.rotation[0] = math::to_radians(rotation[0]);
        component.rotation[1] = math::to_radians(rotation[1]);
        component.rotation[2] = math::to_radians(rotation[2]);

        widgets::Vector3("Scale", component.scale, 1.0f, 0.01f); // #TODO Negative scale
    }

    static void DrawMesh(EditorContext& context, Mesh& component) {
        const bool hasModel{ component.Get() != nullptr };
        std::string name{ component.GetName().empty() ? "None" : component.GetName() };
        if (!hasModel && !component.GetName().empty()) {
            name += " (not loaded)";
        }

        if (const auto path{ widgets::AssetField("Model", ICON_FA_CUBE "  " + name, PathFromUtf8(component.GetFilePath()), hasModel, { ".obj", ".fbx", ".glb", ".gltf", ".ply" }) }) {
            const std::string file{ ToUtf8(*path) };
            if (!component.Get() || component.GetFilePath() != file) {
                component.Load(file);
                actions::CommitEdit(context, "Edit Mesh");
            }
        }
        widgets::Checkbox("Draw", component.toDraw);
    }

    static std::string GetBodyDescription(const RigidBody& body) {
        std::string description{ body.bodyType == PhysicsBodyType::eDynamic ? "Dynamic" : "Static" };
        switch (body.colliderShape) {
        case PhysicsColliderShape::eBox:
            return description + ", box collider";
        case PhysicsColliderShape::eSphere:
            return description + ", sphere collider";
        case PhysicsColliderShape::eCapsule:
            return description + ", capsule collider";
        case PhysicsColliderShape::eMesh:
            return description + ", mesh collider";
        case PhysicsColliderShape::eConvexMesh:
            return description + ", convex mesh collider";
        case PhysicsColliderShape::eInvalid:
            break;
        }
        return description + ", no collider";
    }

    static void DrawRigidBody(EditorContext&, RigidBody& component) {
        widgets::Property("Body");
        ImGui::TextUnformatted(GetBodyDescription(component).c_str());

        if (component.bodyType == PhysicsBodyType::eDynamic) {
            widgets::DragFloat("Mass", component.mass, 0.01f, 0.0f, 1'000.0f);
            widgets::DragFloat("Static Friction", component.staticFriction, 0.001f, 0.0f, 1.0f);
            widgets::DragFloat("Dynamic Friction", component.dynamicFriction, 0.001f, 0.0f, 1.0f);
            widgets::DragFloat("Restitution", component.restitution, 0.001f, 0.0f, 1.0f);
        }

        if (component.colliderShape == PhysicsColliderShape::eBox) {
            widgets::Vector3("Scale", component.scale, 1.0f, 0.01f);
            widgets::Checkbox("Use Model's Scale", component.scaleSameAsModel);
        } else if (component.colliderShape == PhysicsColliderShape::eSphere) {
            widgets::DragFloat("Radius", component.radius, 0.001f, 0.0f, 1'000.0f);
        } else if (component.colliderShape == PhysicsColliderShape::eCapsule) {
            widgets::DragFloat("Radius", component.radius, 0.001f, 0.0f, 1'000.0f);
            widgets::DragFloat("Half Height", component.halfHeight, 0.001f, 0.0f, 1'000.0f);
        }

        if (component.bodyType == PhysicsBodyType::eDynamic) {
            if (widgets::Checkbox("Kinematic", component.isKinematic)) {
                component.SetKinematic(component.isKinematic);
            }
        }
        widgets::Checkbox("Trigger", component.isTrigger);
    }

    static void DrawMaterial(EditorContext&, Material& component) {
        widgets::ColorEdit3("Albedo", component.albedo.data(), ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
        widgets::DragFloat("Roughness", component.roughness, 0.001f, 0.0f, 1.0f);
        widgets::DragFloat("Metallicness", component.metallicness, 0.001f, 0.0f, 1.0f);
        widgets::DragFloat("Transparency", component.transparency, 0.001f, 0.0f, 1.0f);
        widgets::DragFloat("Emissive", component.emissive, 0.01f, 0.0f, 100.0f);
        widgets::DragFloat("Height Scale", component.heightScale, 0.001f, 0.0f, 0.1f);
        widgets::DragFloat("Ambient Occlusion", component.ambientOcclusion, 0.001f, 0.0f, 1.0f);
    }

    static void DrawMaterialTextures(EditorContext& context, MaterialTextures& component) {
        widgets::Checkbox("Enabled", component.enabled);
        constexpr const char* labels[MaterialTextures::eMapCount]{ "Albedo", "Normal", "Height" };
        bool changed{};
        for (std::uint32_t map{}; map != MaterialTextures::eMapCount; ++map) {
            std::string& file{ component.files[map] };
            bool cleared{};
            const std::string name{ ICON_FA_IMAGE "  " + (file.empty() ? std::string{ "None" } : file) };
            if (const auto path{ widgets::AssetField(labels[map], name, context.paths.textures / PathFromUtf8(file), !file.empty(), { ".tga", ".png", ".jpg" }, &cleared) }) {
                file    = ToUtf8(path->lexically_relative(context.paths.textures));
                changed = true;
            }
            if (cleared && !file.empty()) {
                file.clear();
                changed = true;
            }
        }
        if (changed) {
            component.Load();
            actions::CommitEdit(context, "Edit Material Textures");
        }
    }

    static void DrawCamera2D(EditorContext&, Camera2D& component) {
        widgets::Vector3("Eye Position", component.eyePosition);
        widgets::Vector3("Focus Position", component.focusPosition);
        widgets::Vector3("Up Vector", component.upVector);

        constexpr auto clamp{ ImGuiSliderFlags_AlwaysClamp };
        widgets::DragFloat("Left", component.left, 0.01f, -1000.0f, component.right - 0.01f, "%.3f", clamp);
        widgets::DragFloat("Right", component.right, 0.01f, component.left + 0.01f, 1000.0f, "%.3f", clamp);
        widgets::DragFloat("Bottom", component.bottom, 0.01f, -1000.0f, component.top - 0.01f, "%.3f", clamp);
        widgets::DragFloat("Top", component.top, 0.01f, component.bottom + 0.01f, 1000.0f, "%.3f", clamp);
        widgets::DragFloat("Near Z", component.nearZ, 0.01f, 0.1f, component.farZ - 0.01f, "%.3f", clamp);
        widgets::DragFloat("Far Z", component.farZ, 0.1f, component.nearZ + 0.01f, 1000.0f, "%.3f", clamp);

        widgets::Checkbox("Scene Camera", component.isSceneCamera);
        widgets::Checkbox("Runtime Camera", component.isRuntimeCamera);
    }

    static void DrawCamera3D(EditorContext&, Camera3D& component) {
        widgets::Vector3("Eye Position", component.eyePosition);
        widgets::Vector3("Focus Position", component.focusPosition);
        widgets::Vector3("Up Vector", component.upVector);

        constexpr auto clamp{ ImGuiSliderFlags_AlwaysClamp };
        widgets::DragFloat("Field of View", component.fieldOfView, 0.1f, 1.0f, 179.0f, "%.1f", clamp);
        widgets::DragFloat("Aspect Ratio", component.aspectRatio, 0.01f, 1.0f, 100.0f);
        widgets::DragFloat("Near Z", component.nearZ, 0.01f, 0.1f, component.farZ - 0.01f, "%.3f", clamp);
        widgets::DragFloat("Far Z", component.farZ, 0.1f, component.nearZ + 0.01f, 1000.0f, "%.3f", clamp);

        widgets::Checkbox("Scene Camera", component.isSceneCamera);
        widgets::Checkbox("Runtime Camera", component.isRuntimeCamera);
    }

    static void DrawLight(EditorContext&, Light& component) {
        widgets::Property("Type");
        if (ImGui::BeginCombo("##Type", ToString(component.type))) {
            for (const auto type : { Light::Type::ePoint, Light::Type::eSpot }) {
                if (ImGui::Selectable(ToString(type), component.type == type)) {
                    component.type = type;
                }
            }
            ImGui::EndCombo();
        }
        widgets::ColorEdit3("Color", component.color.data(), ImGuiColorEditFlags_Float);

        constexpr auto clamp{ ImGuiSliderFlags_AlwaysClamp };
        widgets::DragFloat("Intensity", component.intensity, 0.1f, 0.0f, 10'000.0f, "%.3f", clamp);
        widgets::DragFloat("Range", component.range, 0.01f, 0.01f, 1'000.0f, "%.3f", clamp);
        if (component.type == Light::Type::eSpot) {
            widgets::DragFloat("Inner Angle", component.innerAngle, 0.1f, 0.0f, component.outerAngle, "%.1f", clamp);
            widgets::DragFloat("Outer Angle", component.outerAngle, 0.1f, component.innerAngle, 89.0f, "%.1f", clamp);
        }
    }

    static void DrawParticleEmitter(EditorContext&, ParticleEmitter& component) {
        constexpr auto clamp{ ImGuiSliderFlags_AlwaysClamp };
        constexpr std::uint32_t maxParticles[]{ 0u, ParticleEmitter::particleLimit };
        widgets::Property("Max Particles");
        ImGui::DragScalar("##Max Particles", ImGuiDataType_U32, &component.maxParticles, 1.0f, &maxParticles[0], &maxParticles[1], "%u", clamp);
        widgets::DragFloat("Spawn Rate", component.spawnRate, 1.0f, 0.0f, 100'000.0f, "%.1f", clamp);
        widgets::DragFloat("Lifetime", component.lifetime, 0.01f, 0.0f, 100.0f, "%.2f", clamp);
        widgets::Vector3("Velocity", component.velocity, 0.0f, 0.01f);
        widgets::DragFloat("Spread", component.spread, 0.01f, 0.0f, 100.0f, "%.2f", clamp);
        widgets::Vector3("Gravity", component.gravity, 0.0f, 0.01f);
        widgets::DragFloat("Size", component.size, 0.001f, 0.0f, 10.0f, "%.3f", clamp);
        widgets::ColorEdit3("Color", component.color.data(), ImGuiColorEditFlags_Float);
        widgets::DragFloat("Intensity", component.intensity, 0.01f, 0.0f, 100.0f, "%.2f", clamp);
    }

    static void DrawSkybox(EditorContext& context, Skybox& component) {
        const std::filesystem::path folder{ context.paths.textures / PathFromUtf8(component.folder) };
        if (const auto path{ widgets::AssetField("Faces", ICON_FA_FOLDER "  " + component.folder, folder, !component.folder.empty(), { ".png", ".jpg" }) }) {
            component.folder = ToUtf8(path->parent_path().lexically_relative(context.paths.textures));
            actions::CommitEdit(context, "Edit Skybox");
        }

        constexpr auto clamp{ ImGuiSliderFlags_AlwaysClamp };
        widgets::DragFloat("Intensity", component.intensity, 0.01f, 0.0f, 100.0f, "%.3f", clamp);
    }

    static void DrawScript(EditorContext& context, Script& component) {
        const std::string name{ component.filePath.empty() ? "None" : component.GetFileName() };
        if (const auto path{ widgets::AssetField("Script", ICON_FA_FILE_CODE "  " + name, PathFromUtf8(component.filePath), !component.filePath.empty(), { ".lua" }) }) {
            const std::string file{ ToUtf8(*path) };
            if (component.filePath != file) {
                actions::FinishEdit(context);
                component = Script{ context.scene->GetState(), file };
                actions::CommitEdit(context, "Edit Script");
            }
        }
    }

    static constexpr ComponentEditor editors[]{
        { .name = "Tag", .icon = ICON_FA_TAG, .has = Has<Tag>, .add = Add<Tag> },
        { .name = "Transform", .icon = ICON_FA_ARROWS_ALT, .has = Has<Transform>, .draw = Draw<Transform, DrawTransform>, .reset = Reset<Transform>, .remove = Remove<Transform>, .add = Add<Transform> },
        { .name = "Mesh", .icon = ICON_FA_CUBE, .has = Has<Mesh>, .draw = Draw<Mesh, DrawMesh>, .remove = Remove<Mesh>, .add = AddMesh },
        { .name = "RigidBody", .icon = ICON_FA_WEIGHT_HANGING, .has = Has<RigidBody>, .draw = Draw<RigidBody, DrawRigidBody>, .remove = Remove<RigidBody>, .addKind = AddKind::eRigidBody, .whyNotAddable = WhyNoRigidBody },
        { .name = "Material", .icon = ICON_FA_PALETTE, .has = Has<Material>, .draw = Draw<Material, DrawMaterial>, .reset = Reset<Material>, .remove = Remove<Material>, .add = Add<Material> },
        { .name = "Material Textures", .icon = ICON_FA_IMAGE, .has = Has<MaterialTextures>, .draw = Draw<MaterialTextures, DrawMaterialTextures>, .remove = Remove<MaterialTextures>, .add = Add<MaterialTextures> },
        { .name = "Camera2D", .icon = ICON_FA_VIDEO, .has = Has<Camera2D>, .draw = Draw<Camera2D, DrawCamera2D>, .reset = Reset<Camera2D>, .remove = Remove<Camera2D>, .add = Add<Camera2D> },
        { .name = "Camera3D", .icon = ICON_FA_VIDEO, .has = Has<Camera3D>, .draw = Draw<Camera3D, DrawCamera3D>, .reset = Reset<Camera3D>, .remove = Remove<Camera3D>, .add = Add<Camera3D> },
        { .name = "Light", .icon = ICON_FA_LIGHTBULB, .has = Has<Light>, .draw = Draw<Light, DrawLight>, .reset = Reset<Light>, .remove = Remove<Light>, .add = Add<Light> },
        { .name = "Particle Emitter", .icon = ICON_FA_FIRE, .has = Has<ParticleEmitter>, .draw = Draw<ParticleEmitter, DrawParticleEmitter>, .reset = Reset<ParticleEmitter>, .remove = Remove<ParticleEmitter>, .add = Add<ParticleEmitter> },
        { .name = "Skybox", .icon = ICON_FA_CLOUD, .has = Has<Skybox>, .draw = Draw<Skybox, DrawSkybox>, .reset = Reset<Skybox>, .remove = Remove<Skybox>, .add = Add<Skybox>, .whyNotAddable = WhyNoSkybox },
        { .name = "Script", .icon = ICON_FA_FILE_CODE, .has = Has<Script>, .draw = Draw<Script, DrawScript>, .remove = Remove<Script>, .addKind = AddKind::eScript },
    };

    std::span<const ComponentEditor> GetComponentEditors() {
        return editors;
    }
} // namespace adh
