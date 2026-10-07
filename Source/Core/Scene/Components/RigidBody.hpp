#pragma once

#include <Vertex.hpp>

#include "Math/Math.hpp"
#include "Mesh.hpp"
#include "Transform.hpp"
#include <Physics/PhysicsWorld.hpp>

namespace adh {
    enum class PhysicsBodyType {
        eStatic,
        eDynamic
    };

    enum class PhysicsColliderShape {
        eBox,
        eSphere,
        eCapsule,
        eMesh,
        eConvexMesh,
        eInvalid
    };

    enum class PhysicsColliderType {
        eCollider,
        eTrigger
    };

    class RigidBody {
      public:
        RigidBody();

        RigidBody(const RigidBody& rhs) = delete;

        RigidBody& operator=(const RigidBody& rhs) = delete;

        RigidBody(RigidBody&& rhs) noexcept;

        RigidBody& operator=(RigidBody&& rhs) noexcept;

        ~RigidBody();

        void Create(std::uint64_t newEntity,
                    float newStaticFriction,
                    float newDynamicFriction,
                    float newRestitution,
                    PhysicsBodyType newBodyType,
                    float newMass,
                    bool newIsKinematic,
                    bool newIsTrigger,
                    bool newScaleSameAsModel,
                    PhysicsColliderShape newColliderShape,
                    PhysicsColliderType newColliderType,
                    const Vector3D& newScale,
                    float newRadius,
                    float newHalfHeight,
                    const Mesh* const mesh = nullptr);

        void OnUpdate(Transform& transform) noexcept;

        void SetTranslation(float x, float y, float z) noexcept;

        Vector3D& GetTranslation() noexcept;

        void SetPosition(float x, float y, float z) noexcept;

        void SetRotation(float x, float y, float z) noexcept;

        void AddRotation(float x, float y, float z) noexcept;

        Vector3D& GetRotation() noexcept;

        void SetVelocity(float x, float y, float z) noexcept;

        void AddVelocity(float x, float y, float z) noexcept;

        Vector3D& GetVelocity() noexcept;

        void SetAngularVelocity(float x, float y, float z) noexcept;

        void AddAngularVelocity(float x, float y, float z) noexcept;

        Vector3D& GetAngularVelocity() noexcept;

        void AddForce(float x, float y, float z) noexcept;

        void AddTorque(float x, float y, float z) noexcept;

        float GetRestitution() const noexcept;

        void SetRestitution(float newRestitution) noexcept;

        float GetStaticFriction() const noexcept;

        void SetStaticFriction(float newStaticFriction) noexcept;

        float GetDynamicFriction() const noexcept;

        void SetDynamicFriction(float newDynamicFriction) noexcept;

        void SetHasGravity(bool hasGravity) noexcept;

        bool GetIsTrigger() const noexcept;

        void SetBodyType() noexcept;

        void SetTrigger(bool newIsTrigger) noexcept;

        float GetMass() const noexcept;

        void SetMass(float newMass) noexcept;

        void SetKinematic(bool newIsKinematic) noexcept;

        void SetGeometry(const physx::PxGeometry& geometry);

        void SetLinearFactor(bool x, bool y, bool z) noexcept;

        void SetAngularFactor(bool x, bool y, bool z) noexcept;

        void ClearForces() noexcept;

        void UpdateGeometry() noexcept;

        void Destroy() noexcept;

      private:
        physx::PxRigidDynamic* SimulatedActor() const noexcept;

        void MoveConstruct(RigidBody&& rhs) noexcept;

        void Clear() noexcept;

      public:
        // Material
        float staticFriction{};
        float dynamicFriction{};
        float restitution{};
        physx::PxMaterial* material;

        // Actor
        bool isKinematic{ false };
        float mass{ 1.0f };
        PhysicsBodyType bodyType{};
        physx::PxRigidActor* actor{};

        // Shape
        PhysicsColliderShape colliderShape{};
        PhysicsColliderType colliderType{};
        physx::PxShape* shape{};
        bool scaleSameAsModel{ true };
        Vector3D scale{};
        float radius{};
        float halfHeight{};

        std::uint64_t entity{};

        bool isTrigger{ false };

        Vector3D velocity{};
        Vector3D angularVelocity{};
        Vector3D translate{};
        Vector3D rotation{};
    };
} // namespace adh
