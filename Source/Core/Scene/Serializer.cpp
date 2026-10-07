#include "Serializer.hpp"
#include "ComponentsSerializer.hpp"
#include "Utility.hpp"

#include <Event/Event.hpp>
#include <Scene/Scene.hpp>
#include <Scripting/Script.hpp>
#include <Utf8.hpp>
#include <Vulkan/Context.hpp>

#include <exception>
#include <fstream>

namespace adh {
    template <typename T>
    static void SerializeComponent(YAML::Emitter& out, const char* name, T func) {
        out << YAML::Key << name;
        out << YAML::BeginMap;
        func();
        out << YAML::EndMap;
    }

    static void LogError(const std::filesystem::path& filePath, const char* what, const std::string& reason) {
        const std::string message{ "[" + ToUtf8(filePath) + "] Failed to " + what + ": " + reason + "\n" };
        EventBus().publish<EditorLogEvent>(EditorLogEvent::Type::eError, message.c_str());
    }

    static std::string AssetPathForScene(const std::string& filePath, const std::filesystem::path& assetDirectory) {
        if (filePath.empty()) {
            return {};
        }
        const auto absoluteFile{ std::filesystem::absolute(PathFromUtf8(filePath)).lexically_normal() };
        const auto absoluteRoot{ std::filesystem::absolute(assetDirectory).lexically_normal() };
        const auto relative{ absoluteFile.lexically_relative(absoluteRoot) };
        if (!relative.empty() && *relative.begin() != "..") {
            return ToUtf8(relative);
        }
        return ToUtf8(absoluteFile);
    }

    static std::string AssetPathFromScene(const std::string& scenePath, const std::filesystem::path& assetDirectory) {
        if (scenePath.empty()) {
            return {};
        }
        return ToUtf8((assetDirectory / PathFromUtf8(scenePath)).lexically_normal());
    }

    Serializer::Serializer(Scene* scene)
        : m_Scene{ scene } {
    }

    void Serializer::Serialize() {
        m_Data = ToText();
    }

    std::string Serializer::ToText() const {
        YAML::Emitter out;

        out << YAML::BeginMap << YAML::Key << "Scene" << YAML::Value << m_Scene->m_Tag;
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

        for (const ecs::Entity entity : m_Scene->GetEntities()) {
            out << YAML::BeginMap;
            out << YAML::Key << "Entity" << YAML::Value << static_cast<ecs::EntityID>(entity);

            if (m_Scene->m_World.has_component<Tag>(entity)) {
                SerializeComponent(out, "Tag", [&]() {
                    auto& tag = m_Scene->m_World.get<Tag>(entity);
                    out << YAML::Key << "tag" << YAML::Value << tag.Get();
                });
            }

            if (m_Scene->m_World.has_component<Transform>(entity)) {
                SerializeComponent(out, "Transform", [&]() {
                    auto& tranfrom = m_Scene->m_World.get<Transform>(entity);
                    out << YAML::Key << "translate" << YAML::Value << tranfrom.translate;
                    out << YAML::Key << "rotation" << YAML::Value << tranfrom.rotation;
                    out << YAML::Key << "scale" << YAML::Value << tranfrom.scale;
                });
            }

            if (m_Scene->m_World.has_component<Mesh>(entity)) {
                SerializeComponent(out, "Mesh", [&]() {
                    auto& mesh = m_Scene->m_World.get<Mesh>(entity);
                    const auto models{ PathFromUtf8(vk::Context::Get()->GetDataDirectory()) / "Assets/Models" };
                    out << YAML::Key << "path" << YAML::Value << AssetPathForScene(mesh.GetFilePath(), models);
                    out << YAML::Key << "draw" << YAML::Value << mesh.toDraw;
                });
            }

            if (m_Scene->m_World.has_component<Material>(entity)) {
                SerializeComponent(out, "Material", [&]() {
                    auto& material = m_Scene->m_World.get<Material>(entity);
                    out << YAML::Key << "roughness" << YAML::Value << material.roughness;
                    out << YAML::Key << "metallicness" << YAML::Value << material.metallicness;
                    out << YAML::Key << "transparency" << YAML::Value << material.transparency;
                    out << YAML::Key << "albedo" << YAML::Value << material.albedo;
                    out << YAML::Key << "emissive" << YAML::Value << material.emissive;
                    out << YAML::Key << "height scale" << YAML::Value << material.heightScale;
                    out << YAML::Key << "ambient occlusion" << YAML::Value << material.ambientOcclusion;
                });
            }

            if (m_Scene->m_World.has_component<Script>(entity)) {
                SerializeComponent(out, "Script", [&]() {
                    auto& script = m_Scene->m_World.get<Script>(entity);
                    if (script.filePath.empty()) {
                        out << YAML::Key << "source" << YAML::Value << script.source;
                    } else {
                        const auto scripts{ PathFromUtf8(vk::Context::Get()->GetDataDirectory()) / "Assets/Scripts" };
                        out << YAML::Key << "path" << YAML::Value << AssetPathForScene(script.filePath, scripts);
                    }
                });
            }

            if (m_Scene->m_World.has_component<MaterialTextures>(entity)) {
                SerializeComponent(out, "MaterialTextures", [&]() {
                    auto& textures = m_Scene->m_World.get<MaterialTextures>(entity);
                    out << YAML::Key << "enabled" << YAML::Value << textures.enabled;
                    for (std::uint32_t map{}; map != MaterialTextures::eMapCount; ++map) {
                        out << YAML::Key << MaterialTextures::mapNames[map] << YAML::Value << textures.files[map];
                    }
                });
            }

            if (m_Scene->m_World.has_component<RigidBody>(entity)) {
                SerializeComponent(out, "RigidBody", [&]() {
                    auto& rigidbody = m_Scene->m_World.get<RigidBody>(entity);
                    out << YAML::Key << "mass" << YAML::Value << rigidbody.mass;
                    out << YAML::Key << "restitution" << YAML::Value << rigidbody.restitution;
                    out << YAML::Key << "static friction" << YAML::Value << rigidbody.staticFriction;
                    out << YAML::Key << "dynamic friction" << YAML::Value << rigidbody.dynamicFriction;
                    out << YAML::Key << "is trigger" << YAML::Value << rigidbody.isTrigger;
                    out << YAML::Key << "is kinematic" << YAML::Value << rigidbody.isKinematic;
                    out << YAML::Key << "scale is same as model" << YAML::Value << rigidbody.scaleSameAsModel;
                    out << YAML::Key << "collider type" << YAML::Value << (uint64_t)rigidbody.colliderType;
                    out << YAML::Key << "collider shape" << YAML::Value << (uint64_t)rigidbody.colliderShape;
                    out << YAML::Key << "body type" << YAML::Value << (uint64_t)rigidbody.bodyType;
                    out << YAML::Key << "entity" << YAML::Value << (uint64_t)rigidbody.entity;
                    out << YAML::Key << "scale" << YAML::Value << rigidbody.scale;
                    out << YAML::Key << "radius" << YAML::Value << rigidbody.radius;
                    out << YAML::Key << "half height" << YAML::Value << rigidbody.halfHeight;
                });
            }

            if (m_Scene->m_World.has_component<Camera2D>(entity)) {
                SerializeComponent(out, "Camera2D", [&]() {
                    auto& camera = m_Scene->m_World.get<Camera2D>(entity);
                    out << YAML::Key << "eye position" << YAML::Value << camera.eyePosition;
                    out << YAML::Key << "focus position" << YAML::Value << camera.focusPosition;
                    out << YAML::Key << "up vector" << YAML::Value << camera.upVector;
                    out << YAML::Key << "left" << YAML::Value << camera.left;
                    out << YAML::Key << "right" << YAML::Value << camera.right;
                    out << YAML::Key << "bottom" << YAML::Value << camera.bottom;
                    out << YAML::Key << "top" << YAML::Value << camera.top;
                    out << YAML::Key << "nearZ" << YAML::Value << camera.nearZ;
                    out << YAML::Key << "farZ" << YAML::Value << camera.farZ;
                    out << YAML::Key << "is runtime camera" << YAML::Value << camera.isRuntimeCamera;
                    out << YAML::Key << "is scene camera" << YAML::Value << camera.isSceneCamera;
                });
            }

            if (m_Scene->m_World.has_component<Camera3D>(entity)) {
                SerializeComponent(out, "Camera3D", [&]() {
                    auto& camera = m_Scene->m_World.get<Camera3D>(entity);
                    out << YAML::Key << "eye position" << YAML::Value << camera.eyePosition;
                    out << YAML::Key << "focus position" << YAML::Value << camera.focusPosition;
                    out << YAML::Key << "up vector" << YAML::Value << camera.upVector;
                    out << YAML::Key << "field of view" << YAML::Value << camera.fieldOfView;
                    out << YAML::Key << "aspect ratio" << YAML::Value << camera.aspectRatio;
                    out << YAML::Key << "nearZ" << YAML::Value << camera.nearZ;
                    out << YAML::Key << "farZ" << YAML::Value << camera.farZ;
                    out << YAML::Key << "is runtime camera" << YAML::Value << camera.isRuntimeCamera;
                    out << YAML::Key << "is scene camera" << YAML::Value << camera.isSceneCamera;
                });
            }

            if (m_Scene->m_World.has_component<Light>(entity)) {
                SerializeComponent(out, "Light", [&]() {
                    auto& light = m_Scene->m_World.get<Light>(entity);
                    out << YAML::Key << "type" << YAML::Value << ToString(light.type);
                    out << YAML::Key << "color" << YAML::Value << light.color;
                    out << YAML::Key << "intensity" << YAML::Value << light.intensity;
                    out << YAML::Key << "range" << YAML::Value << light.range;
                    out << YAML::Key << "inner angle" << YAML::Value << light.innerAngle;
                    out << YAML::Key << "outer angle" << YAML::Value << light.outerAngle;
                });
            }

            if (m_Scene->m_World.has_component<ParticleEmitter>(entity)) {
                SerializeComponent(out, "ParticleEmitter", [&]() {
                    auto& emitter = m_Scene->m_World.get<ParticleEmitter>(entity);
                    out << YAML::Key << "max particles" << YAML::Value << emitter.maxParticles;
                    out << YAML::Key << "spawn rate" << YAML::Value << emitter.spawnRate;
                    out << YAML::Key << "lifetime" << YAML::Value << emitter.lifetime;
                    out << YAML::Key << "velocity" << YAML::Value << emitter.velocity;
                    out << YAML::Key << "spread" << YAML::Value << emitter.spread;
                    out << YAML::Key << "gravity" << YAML::Value << emitter.gravity;
                    out << YAML::Key << "size" << YAML::Value << emitter.size;
                    out << YAML::Key << "color" << YAML::Value << emitter.color;
                    out << YAML::Key << "intensity" << YAML::Value << emitter.intensity;
                });
            }

            if (m_Scene->m_World.has_component<Skybox>(entity)) {
                SerializeComponent(out, "Skybox", [&]() {
                    auto& skybox = m_Scene->m_World.get<Skybox>(entity);
                    out << YAML::Key << "folder" << YAML::Value << skybox.folder;
                    out << YAML::Key << "intensity" << YAML::Value << skybox.intensity;
                });
            }

            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
        out << YAML::EndMap;
        return out.c_str();
    }

    bool Serializer::SerializeToFile(const std::filesystem::path& filePath) {
        std::string text;
        try {
            text = ToText();
        } catch (const std::exception& exception) {
            LogError(filePath, "save the scene", exception.what());
            return false;
        }
        std::ofstream file{ filePath };
        file << text;
        file.close();
        if (!file) {
            LogError(filePath, "save the scene", "the file can't be written");
            return false;
        }
        return true;
    }

    void Serializer::Deserialize() {
        FromText(m_Data);
    }

    void Serializer::FromText(const std::string& text) {
        YAML::Node node{ YAML::Load(text) };
        Deserialize(&node);
    }

    bool Serializer::DeserializeFromFile(const std::filesystem::path& filePath) {
        std::ifstream file{ filePath };
        if (!file) {
            LogError(filePath, "load the scene", "the file can't be read");
            return false;
        }
        YAML::Node node;
        try {
            node = YAML::Load(file);
        } catch (const std::exception& exception) {
            LogError(filePath, "load the scene", exception.what());
            return false;
        }
        if (!node.IsMap() || !node["Scene"].IsScalar()) {
            LogError(filePath, "load the scene", "it isn't a scene file");
            return false;
        }
        const std::string previous{ ToText() };
        try {
            Deserialize(&node);
        } catch (const std::exception& exception) {
            LogError(filePath, "load the scene", exception.what());
            YAML::Node previousNode{ YAML::Load(previous) };
            Deserialize(&previousNode);
            return false;
        }
        return true;
    }

    void Serializer::Deserialize(void* pNode) {
        YAML::Node& node = *(YAML::Node*)(pNode);
        m_Scene->Clear(node["Scene"].as<std::string>());

        auto& world   = m_Scene->m_World;
        auto entities = node["Entities"];
        if (entities) {
            for (auto&& i : entities) {
                ecs::Entity e{ world.create_entity() };

                auto tag = i["Tag"];
                if (tag) {
                    world.add<Tag>(e, std::string(tag["tag"].as<std::string>()));
                }

                auto transform = i["Transform"];
                if (transform) {
                    world.add<Transform>(e,
                                         Transform(transform["translate"].as<Vector3D>(),
                                                   transform["rotation"].as<Vector3D>(),
                                                   transform["scale"].as<Vector3D>()));
                }

                auto mesh = i["Mesh"];
                if (mesh) {
                    auto& m{ world.add<Mesh>(e, Mesh{}) };
                    auto path{ (mesh["path"] ? mesh["path"] : mesh["name"]).as<std::string>() };
                    if (!path.empty()) {
                        const auto models{ PathFromUtf8(vk::Context::Get()->GetDataDirectory()) / "Assets/Models" };
                        m.Load(AssetPathFromScene(path, models));
                    }
                    m.toDraw = mesh["draw"].as<bool>();
                }

                auto material = i["Material"];
                if (material) {
                    auto& m            = world.add<Material>(e, Material{});
                    m.roughness        = material["roughness"].as<float>();
                    m.metallicness     = material["metallicness"].as<float>();
                    m.transparency     = material["transparency"].as<float>();
                    m.albedo           = material["albedo"].as<Vector3D>();
                    m.emissive         = material["emissive"].as<float>(0.0f);
                    m.heightScale      = material["height scale"].as<float>(0.0f);
                    m.ambientOcclusion = material["ambient occlusion"].as<float>(1.0f);
                }

                auto script = i["Script"];
                if (script) {
                    Script component;
                    if (script["source"]) {
                        component.source = script["source"].as<std::string>();
                    } else {
                        const auto scripts{ PathFromUtf8(vk::Context::Get()->GetDataDirectory()) / "Assets/Scripts" };
                        component.filePath = AssetPathFromScene((script["path"] ? script["path"] : script["name"]).as<std::string>(), scripts);
                    }
                    component.Compile(m_Scene->m_State);
                    world.add<Script>(e, std::move(component));
                }

                auto materialTextures = i["MaterialTextures"];
                if (materialTextures) {
                    auto& t   = world.add<MaterialTextures>(e, MaterialTextures{});
                    t.enabled = materialTextures["enabled"].as<bool>(true);
                    for (std::uint32_t map{}; map != MaterialTextures::eMapCount; ++map) {
                        t.files[map] = materialTextures[MaterialTextures::mapNames[map]].as<std::string>("");
                    }
                    t.Load();
                }

                auto rigidbody = i["RigidBody"];
                if (rigidbody && transform) {
                    auto& r{ m_Scene->GetWorld().add<RigidBody>(e, RigidBody{}) };

                    Mesh* p{ nullptr };
                    if (mesh) {
                        auto& m{ m_Scene->GetWorld().get<Mesh>(e) };
                        p = &m;
                    }
                    r.Create(static_cast<std::uint64_t>(e),
                             rigidbody["static friction"].as<float>(),
                             rigidbody["dynamic friction"].as<float>(),
                             rigidbody["restitution"].as<float>(),
                             PhysicsBodyType(rigidbody["body type"].as<int>()),
                             rigidbody["mass"].as<float>(),
                             rigidbody["is kinematic"].as<bool>(),
                             rigidbody["is trigger"].as<bool>(),
                             rigidbody["scale is same as model"].as<bool>(),
                             PhysicsColliderShape(rigidbody["collider shape"].as<int>()),
                             PhysicsColliderType(rigidbody["collider type"].as<int>()),
                             rigidbody["scale"].as<Vector3D>(),
                             rigidbody["radius"].as<float>(),
                             rigidbody["half height"].as<float>(),
                             p);
                }

                auto camera2D = i["Camera2D"];
                if (camera2D) {
                    auto& c           = world.add<Camera2D>(e, Camera2D{});
                    c.eyePosition     = camera2D["eye position"].as<Vector3D>();
                    c.focusPosition   = camera2D["focus position"].as<Vector3D>();
                    c.upVector        = camera2D["up vector"].as<Vector3D>();
                    c.left            = camera2D["left"].as<float>();
                    c.right           = camera2D["right"].as<float>();
                    c.bottom          = camera2D["bottom"].as<float>();
                    c.top             = camera2D["top"].as<float>();
                    c.nearZ           = camera2D["nearZ"].as<float>();
                    c.farZ            = camera2D["farZ"].as<float>();
                    c.isSceneCamera   = camera2D["is scene camera"].as<bool>();
                    c.isRuntimeCamera = camera2D["is runtime camera"].as<bool>();
                }

                auto camera3D = i["Camera3D"];
                if (camera3D) {
                    auto& c           = world.add<Camera3D>(e, Camera3D{});
                    c.eyePosition     = camera3D["eye position"].as<Vector3D>();
                    c.focusPosition   = camera3D["focus position"].as<Vector3D>();
                    c.upVector        = camera3D["up vector"].as<Vector3D>();
                    c.fieldOfView     = camera3D["field of view"].as<float>();
                    c.aspectRatio     = camera3D["aspect ratio"].as<float>();
                    c.nearZ           = camera3D["nearZ"].as<float>();
                    c.farZ            = camera3D["farZ"].as<float>();
                    c.isSceneCamera   = camera3D["is scene camera"].as<bool>();
                    c.isRuntimeCamera = camera3D["is runtime camera"].as<bool>();
                }

                auto light = i["Light"];
                if (light) {
                    auto& l = world.add<Light>(e, Light{});
                    l.SetType(light["type"].as<std::string>().c_str());
                    l.color      = light["color"].as<Vector3D>();
                    l.intensity  = light["intensity"].as<float>();
                    l.range      = light["range"].as<float>();
                    l.innerAngle = light["inner angle"].as<float>();
                    l.outerAngle = light["outer angle"].as<float>();
                }

                auto particleEmitter = i["ParticleEmitter"];
                if (particleEmitter) {
                    auto& p        = world.add<ParticleEmitter>(e, ParticleEmitter{});
                    p.maxParticles = particleEmitter["max particles"].as<std::uint32_t>();
                    p.spawnRate    = particleEmitter["spawn rate"].as<float>();
                    p.lifetime     = particleEmitter["lifetime"].as<float>();
                    p.velocity     = particleEmitter["velocity"].as<Vector3D>();
                    p.spread       = particleEmitter["spread"].as<float>();
                    p.gravity      = particleEmitter["gravity"].as<Vector3D>();
                    p.size         = particleEmitter["size"].as<float>();
                    p.color        = particleEmitter["color"].as<Vector3D>();
                    p.intensity    = particleEmitter["intensity"].as<float>();
                }

                auto skybox = i["Skybox"];
                if (skybox && !m_Scene->GetSkybox()) {
                    auto& s     = world.add<Skybox>(e, Skybox{});
                    s.folder    = skybox["folder"].as<std::string>();
                    s.intensity = skybox["intensity"].as<float>();
                }
            }
        }
    }
} // namespace adh
