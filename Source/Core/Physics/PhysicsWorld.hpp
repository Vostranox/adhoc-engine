#pragma once
#include <PxPhysicsAPI.h>

#include <Math/Math.hpp>
#include <Std/Array.hpp>

#include <Scene/Components/Mesh.hpp>

namespace adh {
    class PhysicsWorld : public physx::PxSimulationEventCallback {
      private:
        inline static PhysicsWorld* s_This;

      public:
        static PhysicsWorld* Get() ADH_NOEXCEPT {
            ADH_THROW(s_This != nullptr, "Physics world pointer is null!");
            return s_This;
        }

      public:
        PhysicsWorld() = default;

        PhysicsWorld(const PhysicsWorld& rhs) = delete;

        PhysicsWorld& operator=(const PhysicsWorld& rhs) = delete;

        PhysicsWorld(PhysicsWorld&& rhs);

        PhysicsWorld& operator=(PhysicsWorld&& rhs);

        ~PhysicsWorld();

        void Create();

        [[nodiscard]] physx::PxMaterial* CreateMaterial(float staticFriction, float dynamicFriction, float restitution);

        [[nodiscard]] physx::PxShape* CreateBoxShape(physx::PxRigidActor* actor, physx::PxMaterial* material, const physx::PxVec3& scale);

        [[nodiscard]] physx::PxShape* CreateSphereShape(physx::PxRigidActor* actor, physx::PxMaterial* material, float radius);

        [[nodiscard]] physx::PxShape* CreateCapsuleShape(physx::PxRigidActor* actor, physx::PxMaterial* material, float radius, float halfHeight);

        [[nodiscard]] physx::PxShape* CreateMeshShape(physx::PxRigidActor* actor, physx::PxMaterial* material, const Mesh& mesh);

        [[nodiscard]] physx::PxShape* CreateConvexMeshShape(physx::PxRigidActor* actor, physx::PxMaterial* material, const Mesh& mesh);

        [[nodiscard]] physx::PxRigidDynamic* CreateDynamicActor();

        [[nodiscard]] physx::PxRigidStatic* CreateStaticActor();

        physx::PxScene* GetScene() noexcept;

        void ResetScene();

        void Destroy() noexcept;

        void StepSimulation(float deltaTime);

        Vector3D& GetGravity() noexcept;

        void SetGravity(const Vector3D& gravity) noexcept;

        std::uint64_t Raycast(const Vector3D& from, const Vector3D& direction, float distance);

        // TODO:
        // void Sweep();

        // TODO:
        // void Overlap();

        void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override;

        void onWake(physx::PxActor** actors, physx::PxU32 count) override;

        void onSleep(physx::PxActor** actors, physx::PxU32 count) override;

        void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;

        void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;

        void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override;

      private:
        void MoveConstruct(PhysicsWorld&& rhs) noexcept;

        void Clear() noexcept;

      private:
        physx::PxDefaultAllocator m_DefaultAllocatorCallback;
        physx::PxDefaultErrorCallback m_DefaultErrorCallback;
        physx::PxDefaultCpuDispatcher* m_Dispatcher;
        physx::PxFoundation* m_Foundation;
        physx::PxCooking* m_Cooking;
        physx::PxPhysics* m_Physics;
        Array<physx::PxScene*> m_Scene;
        std::int32_t m_CurrentScene{ 0 };
        Vector3D m_Gravity{ 0.0f, -9.81f, 0.0f };
    };
} // namespace adh
