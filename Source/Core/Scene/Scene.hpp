#pragma once

#include <Physics/PhysicsWorld.hpp>

#include <adh/entity.hpp>
#include <adh/script.hpp>
#include <filesystem>
#include <string>
#include <vector>

#include "Serializer.hpp"

namespace adh {
    struct Camera3D;
    struct Skybox;

    class Scene {
        friend class Serializer;

      public:
        Scene(std::string tag = "Untitled");

        ecs::World& GetWorld();

        const ecs::World& GetWorld() const;

        script::State& GetState();

        const script::State& GetState() const;

        PhysicsWorld& GetPhysics();

        const PhysicsWorld& GetPhysics() const;

        void ResetPhysicsWorld();

        const std::string& GetTag() const noexcept;

        void SetTag(std::string newTag);

        Skybox* GetSkybox();

        Camera3D* GetSceneCamera();

        std::vector<ecs::Entity> GetEntities();

        void Save();

        bool SaveToFile(const std::filesystem::path& filePath);

        void Load();

        bool LoadFromFile(const std::filesystem::path& filePath);

        std::string SaveToText();

        void LoadFromText(const std::string& text);

        void ResetToDefault();

        ecs::Entity DuplicateEntity(ecs::Entity entity);

      private:
        void Clear(std::string tag);

      private:
        std::string m_Tag;
        PhysicsWorld m_PhysicsWorld;
        script::State m_State;
        ecs::World m_World;
        Serializer m_Serializer;
    };
} // namespace adh
