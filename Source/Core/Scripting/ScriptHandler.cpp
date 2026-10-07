#include "ScriptHandler.hpp"

#include <Input/Input.hpp>
#include <Input/Keycodes.hpp>
#include <Scene/Components.hpp>
#include <Scene/Scene.hpp>
#include <Scripting/Script.hpp>

#include <Audio/Audio.hpp>

#include <Event/Event.hpp>
#include <Vulkan/Context.hpp>

#include <string>
#include <string_view>
#include <utility>

namespace adh {
    struct Reference {
        ecs::Entity entity;
        void* (*resolve)(ecs::Entity entity);
    };

    static char referenceRegistry;

    static std::string Prelude() {
        auto toLua{ [](const auto& names) {
            std::string table{ "{" };
            for (const auto& [name, code] : names) {
                table.append(" ").append(name).append(" = ").append(std::to_string(code)).append(",");
            }
            return table + " }";
        } };
        std::string lua{ "AdHoc = { Key = " + toLua(keyNames) + ", Controller = " + toLua(buttonNames) + ", Global = _G }\n" };
        lua += "package.preload['AdHoc'] = function() return AdHoc end\n";
        return lua;
    }

    static void PushReferences(lua_State* L) {
        lua_rawgetp(L, LUA_REGISTRYINDEX, &referenceRegistry);
        if (!lua_isnil(L, -1)) {
            return;
        }
        lua_pop(L, 1);
        lua_newtable(L);
        lua_newtable(L);
        lua_pushliteral(L, "k");
        lua_setfield(L, -2, "__mode");
        lua_setmetatable(L, -2);
        lua_pushvalue(L, -1);
        lua_rawsetp(L, LUA_REGISTRYINDEX, &referenceRegistry);
    }

    static Reference* FindReference(lua_State* L, int index) {
        if (lua_type(L, index) != LUA_TUSERDATA) {
            return nullptr;
        }
        index = lua_absindex(L, index);
        PushReferences(L);
        lua_pushvalue(L, index);
        lua_rawget(L, -2);
        auto* reference = static_cast<Reference*>(lua_touserdata(L, -1));
        lua_pop(L, 2);
        return reference;
    }

    static int Invoke(lua_State* L) {
        const int count = lua_gettop(L);
        for (int index = 1; index <= count; ++index) {
            if (const Reference* reference = FindReference(L, index)) {
                auto* box   = static_cast<script::BoxBase*>(lua_touserdata(L, index));
                box->object = reference->resolve(reference->entity);
                if (!box->object) {
                    return luaL_error(L, "component reference is no longer alive");
                }
            }
        }
        lua_pushvalue(L, lua_upvalueindex(1));
        lua_insert(L, 1);
        lua_call(L, count, LUA_MULTRET);
        const int results = lua_gettop(L);
        for (int index = 1; index <= results; ++index) {
            if (lua_isfunction(L, index)) {
                lua_pushvalue(L, index);
                lua_pushcclosure(L, &Invoke, 1);
                lua_replace(L, index);
            }
        }
        return results;
    }

    template <typename T>
    static void* ResolveComponent(ecs::Entity entity) {
        auto& world = ScriptHandler::scene->GetWorld();
        return world.has_component<T>(entity) ? &world.get<T>(entity) : nullptr;
    }

    template <typename T>
    static void RegisterType(script::State& state, const char* name) {
        state.register_type<T>(name);
        lua_State* L = state.handle();
        auto* entry  = script::find_entry<T>(L);
        luaL_getmetatable(L, entry->metatable_name.c_str());
        for (const char* method : { "__index", "__newindex" }) {
            lua_getfield(L, -1, method);
            lua_pushcclosure(L, &Invoke, 1);
            lua_setfield(L, -2, method);
        }
        lua_pop(L, 1);
    }

    template <typename T>
    static int PushComponent(lua_State* L, ecs::Entity entity) {
        auto* component = static_cast<T*>(ResolveComponent<T>(entity));
        if (!component) {
            lua_pushnil(L);
            return 1;
        }
        ScriptHandler::scene->GetState().push_object(component);
        PushReferences(L);
        lua_pushvalue(L, -2);
        *static_cast<Reference*>(lua_newuserdatauv(L, sizeof(Reference), 0)) = Reference{ entity, &ResolveComponent<T> };
        lua_rawset(L, -3);
        lua_pop(L, 1);
        return 1;
    }

    int ScriptHandler::GetScene(lua_State* L) {
        lua_pushstring(L, scene->GetTag().data());
        return 1;
    }
    int ScriptHandler::GetThis(lua_State* L) {
        lua_pushinteger(L, currentEntity);
        return 1;
    }

    int ScriptHandler::DeltaTime(lua_State* L) {
        lua_pushnumber(L, deltaTime);
        return 1;
    }

    int ScriptHandler::LoadScene(lua_State* L) {
        loadSceneFilename = luaL_checkstring(L, 1);
        return 0;
    }

    int ScriptHandler::CreateEntity(lua_State* L) {
        auto e{ scene->GetWorld().create_entity() };
        auto [tag, transform, mesh, material] =
            scene->GetWorld().add<Tag, Transform, Mesh, Material>(
                e,
                std::string("New Entity(") + std::to_string(scene->GetWorld().get_entity_count()) + ")",
                Transform{},
                Mesh{},
                Material{});
        mesh.Load(vk::Context::Get()->GetDataDirectory() + "Assets/Models/cube.obj");
        lua_pushinteger(L, static_cast<lua_Integer>(e));
        return 1;
    }

    int ScriptHandler::DestroyEntity(lua_State* L) {
        scene->GetWorld().destroy(static_cast<ecs::Entity>(luaL_checkinteger(L, 1)));
        return 0;
    }

    int ScriptHandler::AddComponent(lua_State* L) {
        const auto entity = static_cast<ecs::Entity>(luaL_checkinteger(L, 1));
        const std::string_view name{ luaL_checkstring(L, 2) };
        auto& world = scene->GetWorld();
        if (!world.is_valid(entity)) {
            std::string err = "AddComponent: entity is not alive!\n";
            EventBus().publish<EditorLogEvent>(EditorLogEvent::Type::eError, err.data());
            return 0;
        }
        if (name == "Transform") {
            if (!world.has_component<Transform>(entity)) {
                world.add<Transform>(entity, Transform{});
            }
        } else if (name == "Material") {
            if (!world.has_component<Material>(entity)) {
                world.add<Material>(entity, Material{});
            }
        } else if (name == "Mesh") {
            if (!world.has_component<Mesh>(entity)) {
                world.add<Mesh>(entity, Mesh{});
            }
        } else if (name == "Tag") {
            if (!world.has_component<Tag>(entity)) {
                world.add<Tag>(entity, Tag{});
            }
        } else if (name == "Camera2D") {
            if (!world.has_component<Camera2D>(entity)) {
                world.add<Camera2D>(entity, Camera2D{});
            }
        } else if (name == "Camera3D") {
            if (!world.has_component<Camera3D>(entity)) {
                world.add<Camera3D>(entity, Camera3D{});
            }
        } else if (name == "Light") {
            if (!world.has_component<Light>(entity)) {
                world.add<Light>(entity, Light{});
            }
        } else if (name == "ParticleEmitter") {
            if (!world.has_component<ParticleEmitter>(entity)) {
                world.add<ParticleEmitter>(entity, ParticleEmitter{});
            }
        } else if (name == "Script") {
            scriptComponentEvent.EmplaceBack([entity, source = std::string{ luaL_optstring(L, 3, "") }]() {
                if (scene->GetWorld().is_valid(entity) && !scene->GetWorld().has_component<Script>(entity)) {
                    Script component;
                    component.source = source;
                    component.Compile(scene->GetState());
                    script::Script instance{ scene->GetWorld().add<Script>(entity, std::move(component)).instance };
                    currentEntity = static_cast<std::uint64_t>(entity);
                    instance.run();
                    if (scene->GetWorld().has_component<Script>(entity)) {
                        instance.call("Start");
                    }
                }
            });
        } else if (name == "MaterialTextures") {
            if (!world.has_component<MaterialTextures>(entity)) {
                auto& textures{ world.add<MaterialTextures>(entity, MaterialTextures{}) };
                for (std::uint32_t map{}; map != MaterialTextures::eMapCount; ++map) {
                    const int argument{ 3 + static_cast<int>(map) };
                    if (lua_isstring(L, argument)) {
                        textures.files[map] = lua_tostring(L, argument);
                    }
                }
                textures.Load();
            }
        } else if (name == "RigidBody") {
            if (!world.has_component<RigidBody>(entity)) {
                if (!lua_isstring(L, 3)) {
                    std::string err{ "Need to specify the rigid body type!" };
                    EventBus().publish<EditorLogEvent>(EditorLogEvent::Type::eError, err.data());
                    return 0;
                }
                const std::string_view type{ lua_tostring(L, 3) };
                const std::string_view staticType{ luaL_optstring(L, 4, "Dynamic") };
                PhysicsColliderShape colliderShape{ PhysicsColliderShape::eInvalid };
                if (type == "Box") {
                    colliderShape = PhysicsColliderShape::eBox;
                } else if (type == "Sphere") {
                    colliderShape = PhysicsColliderShape::eSphere;
                } else if (type == "Capsule") {
                    colliderShape = PhysicsColliderShape::eCapsule;
                } else if (type == "Mesh") {
                    colliderShape = PhysicsColliderShape::eMesh;
                } else if (type == "ConvexMesh") {
                    colliderShape = PhysicsColliderShape::eConvexMesh;
                } else {
                    std::string err{ "Rigid body type not valid" };
                    EventBus().publish<EditorLogEvent>(EditorLogEvent::Type::eLog, err.data());
                    return 0;
                }
                if (world.has_component<Transform>(entity)) {
                    auto& rigidBody{ world.add<RigidBody>(entity, RigidBody{}) };
                    auto& transform{ world.get<Transform>(entity) };

                    Mesh* meshPtr{ nullptr };
                    if ((colliderShape == PhysicsColliderShape::eMesh || colliderShape == PhysicsColliderShape::eConvexMesh) && world.has_component<Mesh>(entity)) {
                        meshPtr = &world.get<Mesh>(entity);
                    }
                    rigidBody.Create(static_cast<std::uint64_t>(entity),
                                     0.5f,
                                     0.5f,
                                     1.0f,
                                     staticType == "Static" ? PhysicsBodyType::eStatic : PhysicsBodyType::eDynamic,
                                     1.0f,
                                     false,
                                     false,
                                     true,
                                     colliderShape,
                                     PhysicsColliderType::eCollider,
                                     transform.scale,
                                     1.0f,
                                     0.5f,
                                     meshPtr);
                    rigidBody.SetPosition(transform.translate.x, transform.translate.y, transform.translate.z);
                    rigidBody.SetRotation(transform.rotation.x, transform.rotation.y, transform.rotation.z);
                }
            }
        } else {
            std::string err = "Component: [" + std::string(name) + "] is invalid!\n";
            EventBus().publish<EditorLogEvent>(EditorLogEvent::Type::eError, err.data());
        }
        return 0;
    }

    int ScriptHandler::RemoveComponent(lua_State* L) {
        const auto entity = static_cast<ecs::Entity>(luaL_checkinteger(L, 1));
        const std::string_view name{ luaL_checkstring(L, 2) };
        auto& world = scene->GetWorld();

        if (name == "Transform") {
            if (world.has_component<Transform>(entity)) {
                world.remove<Transform>(entity);
            }
        } else if (name == "Material") {
            if (world.has_component<Material>(entity)) {
                world.remove<Material>(entity);
            }
        } else if (name == "Mesh") {
            if (world.has_component<Mesh>(entity)) {
                world.remove<Mesh>(entity);
            }
        } else if (name == "Tag") {
            if (world.has_component<Tag>(entity)) {
                world.remove<Tag>(entity);
            }
        } else if (name == "Camera2D") {
            if (world.has_component<Camera2D>(entity)) {
                world.remove<Camera2D>(entity);
            }
        } else if (name == "Camera3D") {
            if (world.has_component<Camera3D>(entity)) {
                world.remove<Camera3D>(entity);
            }
        } else if (name == "Light") {
            if (world.has_component<Light>(entity)) {
                world.remove<Light>(entity);
            }
        } else if (name == "ParticleEmitter") {
            if (world.has_component<ParticleEmitter>(entity)) {
                world.remove<ParticleEmitter>(entity);
            }
        } else if (name == "RigidBody") {
            if (world.has_component<RigidBody>(entity)) {
                world.remove<RigidBody>(entity);
            }
        } else if (name == "Script") {
            if (world.has_component<Script>(entity)) {
                scriptComponentEvent.EmplaceBack([entity]() {
                    if (scene->GetWorld().has_component<Script>(entity)) {
                        scene->GetWorld().remove<Script>(entity);
                    }
                });
            }
        } else if (name == "MaterialTextures") {
            if (world.has_component<MaterialTextures>(entity)) {
                world.remove<MaterialTextures>(entity);
            }
        } else {
            std::string err = "Component: [" + std::string(name) + "] is invalid!\n";
            EventBus().publish<EditorLogEvent>(EditorLogEvent::Type::eError, err.data());
        }

        return 0;
    }

    int ScriptHandler::GetComponent(lua_State* L) {
        const auto entity = static_cast<ecs::Entity>(luaL_checkinteger(L, 1));
        const std::string_view name{ luaL_checkstring(L, 2) };

        if (name == "Transform") {
            return PushComponent<Transform>(L, entity);
        } else if (name == "Material") {
            return PushComponent<Material>(L, entity);
        } else if (name == "Mesh") {
            return PushComponent<Mesh>(L, entity);
        } else if (name == "Script") {
            return PushComponent<Script>(L, entity);
        } else if (name == "Tag") {
            return PushComponent<Tag>(L, entity);
        } else if (name == "Camera2D") {
            return PushComponent<Camera2D>(L, entity);
        } else if (name == "Camera3D") {
            return PushComponent<Camera3D>(L, entity);
        } else if (name == "Light") {
            return PushComponent<Light>(L, entity);
        } else if (name == "ParticleEmitter") {
            return PushComponent<ParticleEmitter>(L, entity);
        } else if (name == "RigidBody") {
            return PushComponent<RigidBody>(L, entity);
        }
        std::string err = "Component: [" + std::string(name) + "] is invalid!\n";
        EventBus().publish<EditorLogEvent>(EditorLogEvent::Type::eLog, err.data());
        return 0;
    }

    int ScriptHandler::GetInput(lua_State*) {
        return scene->GetState().push_object(input);
    }

    int ScriptHandler::LogMessage(lua_State* L) {
        if (lua_isstring(L, 1)) {
            std::string msg{ std::string(lua_tostring(L, 1)) + "\n" };
            EventBus().publish<EditorLogEvent>(EditorLogEvent::Type::eLog, msg.data());
        }
        return 0;
    }

    int ScriptHandler::LogError(lua_State* L) {
        if (lua_isstring(L, 1)) {
            std::string err{ std::string(lua_tostring(L, 1)) + "\n" };
            EventBus().publish<EditorLogEvent>(EditorLogEvent::Type::eError, err.data());
        }
        return 0;
    }

    std::uint64_t ScriptHandler::Raycast(const Vector3D& from, const Vector3D& direction, float distance) {
        return scene->GetPhysics().Raycast(from, direction, distance);
    }

    int ScriptHandler::SetGravity(lua_State* L) {
        auto x = (float)lua_tonumber(L, 1);
        auto y = (float)lua_tonumber(L, 2);
        auto z = (float)lua_tonumber(L, 3);
        scene->GetPhysics().SetGravity(Vector3D{ x, y, z });
        return 0;
    }

    int ScriptHandler::FindEntity(lua_State* L) {
        const std::string_view name{ luaL_checkstring(L, 1) };
        for (auto entity : scene->GetEntities()) {
            if (scene->GetWorld().has_component<Tag>(entity) && scene->GetWorld().get<Tag>(entity).tag == name) {
                lua_pushinteger(L, static_cast<lua_Integer>(entity));
                return 1;
            }
        }
        lua_pushinteger(L, 0);
        return 1;
    }

    // TODO:
    int ScriptHandler::SerializeField(lua_State*) {
        return 0;
    }

    void ScriptHandler::RegisterBindings() {
        auto& state = scene->GetState();

        lua_State* L = state.handle();
        luaL_loadstring(L, Prelude().c_str());
        ADH_THROW(lua_pcall(L, 0, 0, 0) == LUA_OK, lua_tostring(L, -1));

        state.set_error_handler([](const std::string& message) {
            EventBus().publish<EditorLogEvent>(EditorLogEvent::Type::eError, (message + "\n").c_str());
        });

        state.register_raw_function("GetScene", ScriptHandler::GetScene);
        state.register_raw_function("LoadScene", ScriptHandler::LoadScene);
        state.register_raw_function("CreateEntity", ScriptHandler::CreateEntity);
        state.register_raw_function("DestroyEntity", ScriptHandler::DestroyEntity);
        state.register_raw_function("AddComponent", ScriptHandler::AddComponent);
        state.register_raw_function("RemoveComponent", ScriptHandler::RemoveComponent);
        state.register_raw_function("GetComponent", ScriptHandler::GetComponent);
        state.register_raw_function("GetInput", ScriptHandler::GetInput);
        state.register_raw_function("GetThis", ScriptHandler::GetThis);
        state.register_raw_function("LogMessage", ScriptHandler::LogMessage);
        state.register_raw_function("LogError", ScriptHandler::LogError);
        state.register_raw_function("DeltaTime", ScriptHandler::DeltaTime);
        state.register_function("Raycast", &ScriptHandler::Raycast);
        state.register_raw_function("SetGravity", ScriptHandler::SetGravity);
        state.register_raw_function("FindEntity", ScriptHandler::FindEntity);
        state.register_raw_function("SerializeField", ScriptHandler::SerializeField);

        state.register_type<Input>("Input");
        state.register_method<Input>("GetKey", &Input::RepeatGetKey);
        state.register_method<Input>("GetKeyDown", &Input::GetKeyDown);
        state.register_method<Input>("GetKeyUp", &Input::GetKeyUp);
        state.register_method<Input>("GetButton", &Input::RepeatGetButton);
        state.register_method<Input>("GetButtonDown", &Input::GetButtonDown);
        state.register_method<Input>("GetButtonUp", &Input::GetButtonUp);
        state.register_method<Input>("SetControllerVibration", &Input::SetControllerVibration);
        state.register_method<Input>("GetMousePositionX", &Input::GetMousePositionX);
        state.register_method<Input>("GetMousePositionY", &Input::GetMousePositionY);
        state.register_method<Input>("GetLeftMouseButtonDown", &Input::GetLeftMouseButtonDown);
        state.register_method<Input>("GetLeftMouseButtonUp", &Input::GetLeftMouseButtonUp);
        state.register_method<Input>("GetRightMouseButtonDown", &Input::GetRightMouseButtonDown);
        state.register_method<Input>("GetRightMouseButtonUp", &Input::GetRightMouseButtonUp);
        state.register_method<Input>("GetMiddleMouseButtonDown", &Input::GetMiddleMouseButtonDown);
        state.register_method<Input>("GetMiddleMouseButtonUp", &Input::GetMiddleMouseButtonUp);
        state.register_method<Input>("GetMouseWheelDelta", &Input::GetMouseWheelDelta);

        state.register_type<Vector2D>("Vector2D");
        state.register_constructor<Vector2D>("new");
        state.register_variable<Vector2D>("x", &Vector2D::x);
        state.register_variable<Vector2D>("y", &Vector2D::y);

        state.register_type<Vector3D>("Vector3D");
        state.register_constructor<Vector3D>("new");
        state.register_variable<Vector3D>("x", &Vector3D::x);
        state.register_variable<Vector3D>("y", &Vector3D::y);
        state.register_variable<Vector3D>("z", &Vector3D::z);
        state.register_method<Vector3D>("Cross", &Vector3D::cross);

        state.register_method<Vector3D, Vector3D&, const Vector3D&>("Rotate", &Vector3D::rotate);

        state.register_type<Vector4D>("Vector4D");
        state.register_constructor<Vector4D>("new");
        state.register_variable<Vector4D>("x", &Vector4D::x);
        state.register_variable<Vector4D>("y", &Vector4D::y);
        state.register_variable<Vector4D>("z", &Vector4D::z);
        state.register_variable<Vector4D>("w", &Vector4D::w);

        state.register_type<xmm::Vector>("XmmVector");
        state.register_constructor<xmm::Vector>("new");
        state.register_variable<xmm::Vector>("x", &xmm::Vector::x);
        state.register_variable<xmm::Vector>("y", &xmm::Vector::y);
        state.register_variable<xmm::Vector>("z", &xmm::Vector::z);
        state.register_variable<xmm::Vector>("w", &xmm::Vector::w);

        state.register_type<Quaternion<float>>("Quaternion");
        state.register_constructor<Quaternion<float>>("new");
        state.register_variable<Quaternion<float>>("x", &Quaternion<float>::x);
        state.register_variable<Quaternion<float>>("y", &Quaternion<float>::y);
        state.register_variable<Quaternion<float>>("z", &Quaternion<float>::z);
        state.register_variable<Quaternion<float>>("w", &Quaternion<float>::w);

        RegisterType<Tag>(state, "Tag");
        state.register_constructor<Tag>("new");
        state.register_method<Tag>("Get", &Tag::Get);
        state.register_method<Tag>("Set", &Tag::Set);

        RegisterType<Transform>(state, "Transform");
        state.register_constructor<Transform>("new");
        state.register_variable<Transform>("translate", &Transform::translate);
        state.register_variable<Transform>("scale", &Transform::scale);
        state.register_variable<Transform>("rotation", &Transform::rotation);
        state.register_method<Transform>("GetForward", &Transform::GetForward);

        RegisterType<Material>(state, "Material");
        state.register_constructor<Material>("new");
        state.register_variable<Material>("roughness", &Material::roughness);
        state.register_variable<Material>("metallicness", &Material::metallicness);
        state.register_variable<Material>("transparency", &Material::transparency);
        state.register_variable<Material>("albedo", &Material::albedo);
        state.register_variable<Material>("emissive", &Material::emissive);
        state.register_variable<Material>("heightScale", &Material::heightScale);
        state.register_variable<Material>("ambientOcclusion", &Material::ambientOcclusion);

        RegisterType<Mesh>(state, "Mesh");
        state.register_method<Mesh>("GetIndexCount", &Mesh::GetIndexCount);
        state.register_method<Mesh>("Load", &Mesh::Load2);
        state.register_variable<Mesh>("toDraw", &Mesh::toDraw);

        RegisterType<Camera2D>(state, "Camera2D");
        state.register_constructor<Camera2D>("new");
        state.register_variable<Camera2D>("eyePosition", &Camera2D::eyePosition);
        state.register_variable<Camera2D>("focusPosition", &Camera2D::focusPosition);
        state.register_variable<Camera2D>("upVector", &Camera2D::upVector);
        state.register_variable<Camera2D>("left", &Camera2D::left);
        state.register_variable<Camera2D>("right", &Camera2D::right);
        state.register_variable<Camera2D>("bottom", &Camera2D::bottom);
        state.register_variable<Camera2D>("top", &Camera2D::top);
        state.register_variable<Camera2D>("nearZ", &Camera2D::nearZ);
        state.register_variable<Camera2D>("farZ", &Camera2D::farZ);
        state.register_variable<Camera2D>("isSceneCamera", &Camera2D::isSceneCamera);
        state.register_variable<Camera2D>("isGameCamera", &Camera2D::isRuntimeCamera);

        RegisterType<Camera3D>(state, "Camera3D");
        state.register_constructor<Camera3D>("new");
        state.register_variable<Camera3D>("eyePosition", &Camera3D::eyePosition);
        state.register_variable<Camera3D>("focusPosition", &Camera3D::focusPosition);
        state.register_variable<Camera3D>("upVector", &Camera3D::upVector);
        state.register_variable<Camera3D>("fieldOfView", &Camera3D::fieldOfView);
        state.register_variable<Camera3D>("nearZ", &Camera3D::nearZ);
        state.register_variable<Camera3D>("farZ", &Camera3D::farZ);
        state.register_variable<Camera3D>("isSceneCamera", &Camera3D::isSceneCamera);
        state.register_variable<Camera3D>("isGameCamera", &Camera3D::isRuntimeCamera);

        RegisterType<Light>(state, "Light");
        state.register_constructor<Light>("new");
        state.register_method<Light>("GetType", &Light::GetType);
        state.register_method<Light>("SetType", &Light::SetType);
        state.register_variable<Light>("color", &Light::color);
        state.register_variable<Light>("intensity", &Light::intensity);
        state.register_variable<Light>("range", &Light::range);
        state.register_variable<Light>("innerAngle", &Light::innerAngle);
        state.register_variable<Light>("outerAngle", &Light::outerAngle);

        RegisterType<ParticleEmitter>(state, "ParticleEmitter");
        state.register_constructor<ParticleEmitter>("new");
        state.register_variable<ParticleEmitter>("maxParticles", &ParticleEmitter::maxParticles);
        state.register_variable<ParticleEmitter>("spawnRate", &ParticleEmitter::spawnRate);
        state.register_variable<ParticleEmitter>("lifetime", &ParticleEmitter::lifetime);
        state.register_variable<ParticleEmitter>("velocity", &ParticleEmitter::velocity);
        state.register_variable<ParticleEmitter>("spread", &ParticleEmitter::spread);
        state.register_variable<ParticleEmitter>("gravity", &ParticleEmitter::gravity);
        state.register_variable<ParticleEmitter>("size", &ParticleEmitter::size);
        state.register_variable<ParticleEmitter>("color", &ParticleEmitter::color);
        state.register_variable<ParticleEmitter>("intensity", &ParticleEmitter::intensity);

        RegisterType<RigidBody>(state, "RigidBody");
        state.register_constructor<RigidBody>("new");
        state.register_variable<RigidBody>("mass", &RigidBody::mass);
        state.register_variable<RigidBody>("restitution", &RigidBody::restitution);
        state.register_variable<RigidBody>("dynamicFriction", &RigidBody::dynamicFriction);
        state.register_variable<RigidBody>("staticFriction", &RigidBody::staticFriction);
        state.register_variable<RigidBody>("entity", &RigidBody::entity);
        state.register_variable<RigidBody>("isKinematic", &RigidBody::isKinematic);
        state.register_variable<RigidBody>("isTrigger", &RigidBody::isTrigger);
        state.register_variable<RigidBody>("velocity", &RigidBody::velocity);
        state.register_variable<RigidBody>("translate", &RigidBody::translate);
        state.register_variable<RigidBody>("rotation", &RigidBody::rotation);
        state.register_variable<RigidBody>("scale", &RigidBody::scale);
        state.register_variable<RigidBody>("angularVelocity", &RigidBody::angularVelocity);
        state.register_variable<RigidBody>("radius", &RigidBody::radius);
        state.register_variable<RigidBody>("halfHeight", &RigidBody::halfHeight);
        state.register_method<RigidBody>("GetVelocity", &RigidBody::GetVelocity);
        state.register_method<RigidBody>("GetAngularVelocity", &RigidBody::GetAngularVelocity);
        state.register_method<RigidBody>("SetLinearFactor", &RigidBody::SetLinearFactor);
        state.register_method<RigidBody>("SetAngularFactor", &RigidBody::SetAngularFactor);
        state.register_method<RigidBody>("SetVelocity", &RigidBody::SetVelocity);
        state.register_method<RigidBody>("AddVelocity", &RigidBody::AddVelocity);
        state.register_method<RigidBody>("SetAngularVelocity", &RigidBody::SetAngularVelocity);
        state.register_method<RigidBody>("AddAngularVelocity", &RigidBody::AddAngularVelocity);
        state.register_method<RigidBody>("GetIsTrigger", &RigidBody::GetIsTrigger);
        state.register_method<RigidBody>("SetTrigger", &RigidBody::SetTrigger);
        state.register_method<RigidBody>("SetKinematic", &RigidBody::SetKinematic);
        state.register_method<RigidBody>("SetTranslation", &RigidBody::SetTranslation);
        state.register_method<RigidBody>("GetTranslation", &RigidBody::GetTranslation);
        state.register_method<RigidBody>("SetMass", &RigidBody::SetMass);
        state.register_method<RigidBody>("GetMass", &RigidBody::GetMass);
        state.register_method<RigidBody>("SetPosition", &RigidBody::SetPosition);
        state.register_method<RigidBody>("SetRotation", &RigidBody::SetRotation);
        state.register_method<RigidBody>("AddRotation", &RigidBody::AddRotation);
        state.register_method<RigidBody>("GetRotation", &RigidBody::GetRotation);
        state.register_method<RigidBody>("ClearForces", &RigidBody::ClearForces);
        state.register_method<RigidBody>("AddForce", &RigidBody::AddForce);
        state.register_method<RigidBody>("AddTorque", &RigidBody::AddTorque);
        state.register_method<RigidBody>("GetRestitution", &RigidBody::GetRestitution);
        state.register_method<RigidBody>("SetRestitution", &RigidBody::SetRestitution);
        state.register_method<RigidBody>("GetStaticFriction", &RigidBody::GetStaticFriction);
        state.register_method<RigidBody>("SetStaticFriction", &RigidBody::SetStaticFriction);
        state.register_method<RigidBody>("GetDynamicFriction", &RigidBody::GetDynamicFriction);
        state.register_method<RigidBody>("SetDynamicFriction", &RigidBody::SetDynamicFriction);
        state.register_method<RigidBody>("UpdateGeometry", &RigidBody::UpdateGeometry);
        state.register_method<RigidBody>("SetHasGravity", &RigidBody::SetHasGravity);

        state.register_type<Audio>("Audio");
        state.register_constructor<Audio>("new");
        state.register_method<Audio>("Create", &Audio::Create2);
        state.register_method<Audio>("Play", &Audio::Play);
        state.register_method<Audio>("Stop", &Audio::Stop);
        state.register_method<Audio>("Pause", &Audio::Pause);
        state.register_method<Audio>("Loop", &Audio::Loop);
        state.register_method<Audio>("IsPlaying", &Audio::IsPlaying);

        RegisterType<Script>(state, "Script");
    }
} // namespace adh
