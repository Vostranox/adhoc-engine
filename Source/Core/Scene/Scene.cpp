#include "Scene.hpp"
#include <Math/Math.hpp>
#include <Scene/Components.hpp>
#include <Scripting/Script.hpp>
#include <Scripting/ScriptHandler.hpp>

#include <algorithm>
#include <utility>

namespace adh {
    Scene::Scene(std::string tag)
        : m_Tag{ Move(tag) },
          m_State{ script::new_state() },
          m_Serializer(this) {
        m_PhysicsWorld.Create();
    }

    ecs::World& Scene::GetWorld() {
        return m_World;
    }

    const ecs::World& Scene::GetWorld() const {
        return m_World;
    }

    script::State& Scene::GetState() {
        return m_State;
    }

    const script::State& Scene::GetState() const {
        return m_State;
    }

    PhysicsWorld& Scene::GetPhysics() {
        return m_PhysicsWorld;
    }

    const PhysicsWorld& Scene::GetPhysics() const {
        return m_PhysicsWorld;
    }

    void Scene::ResetPhysicsWorld() {
        m_World.get_system<Transform, RigidBody>().for_each([&](Transform& transform, RigidBody& rigidBody) {
            if (rigidBody.bodyType == PhysicsBodyType::eDynamic) {
                rigidBody.SetKinematic(rigidBody.isKinematic);
                rigidBody.ClearForces();
                physx::PxRigidBodyExt::updateMassAndInertia(*static_cast<physx::PxRigidDynamic*>(rigidBody.actor), rigidBody.mass);
            }

            rigidBody.material->setStaticFriction(rigidBody.staticFriction);
            rigidBody.material->setDynamicFriction(rigidBody.dynamicFriction);
            rigidBody.material->setRestitution(rigidBody.restitution);

            if (rigidBody.colliderShape == PhysicsColliderShape::eBox) {
                if (rigidBody.scaleSameAsModel) {
                    rigidBody.SetGeometry(physx::PxBoxGeometry{ transform.scale.x, transform.scale.y, transform.scale.z });
                } else {
                    rigidBody.SetGeometry(physx::PxBoxGeometry{ rigidBody.scale.x, rigidBody.scale.y, rigidBody.scale.z });
                }
            } else if (rigidBody.colliderShape == PhysicsColliderShape::eSphere) {
                rigidBody.radius = (transform.scale[0] + transform.scale[1] + transform.scale[2]) / 3;
                rigidBody.SetGeometry(physx::PxSphereGeometry{ rigidBody.radius });
            } else if (rigidBody.colliderShape == PhysicsColliderShape::eCapsule) {
                rigidBody.radius = (transform.scale[0] + transform.scale[1] + transform.scale[2]) / 3;
                rigidBody.SetGeometry(physx::PxCapsuleGeometry{ rigidBody.radius, rigidBody.halfHeight });
            } else if (rigidBody.colliderShape == PhysicsColliderShape::eMesh || rigidBody.colliderShape == PhysicsColliderShape::eConvexMesh) {
                rigidBody.scale = transform.scale;
                rigidBody.UpdateGeometry();
            }
            rigidBody.SetTrigger(rigidBody.isTrigger);

            physx::PxTransform t;
            t.p = physx::PxVec3{ transform.translate.x, transform.translate.y, transform.translate.z };
            Quaternion<float> qq(transform.rotation);
            physx::PxQuat q(qq.x, qq.y, qq.z, qq.w);
            t.q = q;
            rigidBody.actor->setGlobalPose(t);
        });
    }

    const std::string& Scene::GetTag() const noexcept {
        return m_Tag;
    }

    void Scene::SetTag(std::string newTag) {
        m_Tag = Move(newTag);
    }

    Skybox* Scene::GetSkybox() {
        Skybox* skybox{};
        m_World.get_system<Skybox>().for_each([&](Skybox& candidate) {
            skybox = &candidate;
        });
        return skybox;
    }

    Camera3D* Scene::GetSceneCamera() {
        Camera3D* sceneCamera{};
        m_World.get_system<Camera3D>().for_each([&](Camera3D& camera) {
            if (camera.isSceneCamera) {
                sceneCamera = &camera;
            }
        });
        return sceneCamera;
    }

    std::vector<ecs::Entity> Scene::GetEntities() {
        std::vector<ecs::Entity> entities;
        entities.reserve(m_World.get_entity_count());
        m_World.for_each([&](ecs::Entity entity) {
            entities.push_back(entity);
        });
        std::sort(entities.begin(), entities.end());
        return entities;
    }

    void Scene::Save() {
        m_Serializer.Serialize();
    }

    bool Scene::SaveToFile(const std::filesystem::path& filePath) {
        return m_Serializer.SerializeToFile(filePath);
    }

    void Scene::Load() {
        m_Serializer.Deserialize();
    }

    bool Scene::LoadFromFile(const std::filesystem::path& filePath) {
        return m_Serializer.DeserializeFromFile(filePath);
    }

    std::string Scene::SaveToText() {
        return m_Serializer.ToText();
    }

    void Scene::LoadFromText(const std::string& text) {
        m_Serializer.FromText(text);
    }

    void Scene::ResetToDefault() {
        Clear("Untitled");

        const ecs::Entity runtimeCamera{ m_World.create_entity() };
        m_World.add<Tag>(runtimeCamera, "Runtime Camera");
        auto& gameCamera{ m_World.add<Camera3D>(runtimeCamera, Camera3D{}) };
        gameCamera.isRuntimeCamera = true;
        gameCamera.eyePosition.z   = -8;

        const ecs::Entity sceneCamera{ m_World.create_entity() };
        m_World.add<Tag>(sceneCamera, "Main Camera");
        auto& editorCamera{ m_World.add<Camera3D>(sceneCamera, Camera3D{}) };
        editorCamera.isSceneCamera = true;
        editorCamera.eyePosition   = Vector3D{ -7.0f, 8.0f, -14.0f };
        editorCamera.focusPosition = Vector3D{ 0.0f, 0.0f, 0.0f };
    }

    ecs::Entity Scene::DuplicateEntity(ecs::Entity entity) {
        const ecs::Entity copy{ m_World.create_entity() };

        if (m_World.has_component<Tag>(entity)) {
            m_World.add<Tag>(copy, m_World.get<Tag>(entity));
        }
        if (m_World.has_component<Transform>(entity)) {
            m_World.add<Transform>(copy, m_World.get<Transform>(entity));
        }
        if (m_World.has_component<Material>(entity)) {
            m_World.add<Material>(copy, m_World.get<Material>(entity));
        }
        if (m_World.has_component<Camera2D>(entity)) {
            m_World.add<Camera2D>(copy, m_World.get<Camera2D>(entity));
        }
        if (m_World.has_component<Camera3D>(entity)) {
            m_World.add<Camera3D>(copy, m_World.get<Camera3D>(entity));
        }
        if (m_World.has_component<Light>(entity)) {
            m_World.add<Light>(copy, m_World.get<Light>(entity));
        }
        if (m_World.has_component<ParticleEmitter>(entity)) {
            m_World.add<ParticleEmitter>(copy, m_World.get<ParticleEmitter>(entity));
        }
        if (m_World.has_component<Mesh>(entity)) {
            m_World.add<Mesh>(copy, m_World.get<Mesh>(entity));
        }
        if (m_World.has_component<RigidBody>(entity) && m_World.has_component<Transform>(entity)) {
            m_World.add<RigidBody>(copy, RigidBody{});
            const RigidBody& body{ m_World.get<RigidBody>(entity) };
            const Mesh* mesh{ m_World.has_component<Mesh>(copy) ? &m_World.get<Mesh>(copy) : nullptr };
            RigidBody& copyBody{ m_World.get<RigidBody>(copy) };
            copyBody.Create(static_cast<std::uint64_t>(copy),
                            body.staticFriction,
                            body.dynamicFriction,
                            body.restitution,
                            body.bodyType,
                            body.mass,
                            body.isKinematic,
                            body.isTrigger,
                            body.scaleSameAsModel,
                            body.colliderShape,
                            body.colliderType,
                            body.scale,
                            body.radius,
                            body.halfHeight,
                            mesh);
            const Transform& transform{ m_World.get<Transform>(copy) };
            copyBody.SetPosition(transform.translate.x, transform.translate.y, transform.translate.z);
            copyBody.SetRotation(transform.rotation.x, transform.rotation.y, transform.rotation.z);
        }
        if (m_World.has_component<Script>(entity)) {
            const auto& original = m_World.get<Script>(entity);
            Script script;
            script.filePath = original.filePath;
            script.source   = original.source;
            script.Compile(m_State);
            m_World.add<Script>(copy, std::move(script));
        }
        if (m_World.has_component<MaterialTextures>(entity)) {
            MaterialTextures& textures{ m_World.add<MaterialTextures>(copy, MaterialTextures{}) };
            const MaterialTextures& original{ m_World.get<MaterialTextures>(entity) };
            textures.files   = original.files;
            textures.enabled = original.enabled;
            textures.Load();
        }
        return copy;
    }

    void Scene::Clear(std::string tag) {
        m_Tag = std::move(tag);
        ScriptHandler::loadSceneFilename.reset();
        ScriptHandler::scriptComponentEvent.Clear();
        m_World.reset();
        m_PhysicsWorld.ResetScene();
        m_State              = script::new_state();
        ScriptHandler::scene = this;
        ScriptHandler::RegisterBindings();
    }
} // namespace adh
