#include "RigidBody.hpp"
#include "foundation/PxVec3.h"
#include <Event/Event.hpp>
#include <Math/Math.hpp>
#include <iostream>

namespace adh {
    RigidBody::RigidBody() : material{},
                             actor{},
                             shape{} {}

    RigidBody::RigidBody(RigidBody&& rhs) noexcept {
        MoveConstruct(Move(rhs));
    }

    RigidBody& RigidBody::operator=(RigidBody&& rhs) noexcept {
        Clear();
        MoveConstruct(Move(rhs));
        return *this;
    }

    RigidBody::~RigidBody() {
        Clear();
    }

    void RigidBody::Create(std::uint64_t newEntity,
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
                           const Mesh* const mesh) {
        entity           = newEntity;
        scaleSameAsModel = newScaleSameAsModel;

        // Material
        staticFriction  = newStaticFriction;
        dynamicFriction = newDynamicFriction;
        restitution     = newRestitution;
        material        = PhysicsWorld::Get()->CreateMaterial(newStaticFriction, newDynamicFriction, newRestitution);

        // Actor
        bodyType    = newBodyType;
        mass        = newMass;
        isKinematic = newIsKinematic;
        switch (newBodyType) {
        case PhysicsBodyType::eDynamic:
            {
                actor = PhysicsWorld::Get()->CreateDynamicActor();
                physx::PxRigidBodyExt::updateMassAndInertia(*static_cast<physx::PxRigidDynamic*>(actor), newMass);
                static_cast<physx::PxRigidDynamic*>(actor)->setRigidBodyFlag(physx::PxRigidBodyFlag::Enum::eKINEMATIC, newIsKinematic);
                break;
            }
        case PhysicsBodyType::eStatic:
            {
                actor = PhysicsWorld::Get()->CreateStaticActor();
                break;
            }
        }

        // Shape
        colliderShape = newColliderShape;
        colliderType  = newColliderType;
        isTrigger     = newIsTrigger;
        scale         = newScale;
        radius        = newRadius;
        halfHeight    = newHalfHeight;

        ADH_THROW(newColliderShape != PhysicsColliderShape::eInvalid, "Invalid collider shape!");
        switch (newColliderShape) {
        case PhysicsColliderShape::eBox:
            {
                shape = PhysicsWorld::Get()->CreateBoxShape(actor, material, physx::PxVec3(newScale.x, newScale.y, newScale.z));
                break;
            }
        case PhysicsColliderShape::eSphere:
            {
                shape = PhysicsWorld::Get()->CreateSphereShape(actor, material, newRadius);
                break;
            }
        case PhysicsColliderShape::eCapsule:
            {
                shape = PhysicsWorld::Get()->CreateCapsuleShape(actor, material, newRadius, newHalfHeight);
                break;
            }
        case PhysicsColliderShape::eMesh:
            {
                if (mesh && mesh->Get()) {
                    shape = PhysicsWorld::Get()->CreateMeshShape(actor, material, *mesh);
                }
                break;
            }
        case PhysicsColliderShape::eConvexMesh:
            {
                if (mesh && mesh->Get()) {
                    shape = PhysicsWorld::Get()->CreateConvexMeshShape(actor, material, *mesh);
                }
                break;
            }
        case PhysicsColliderShape::eInvalid:
            {
                break;
            }
        }
        SetTrigger(newIsTrigger);
        if (shape) {
            physx::PxFilterData filter;
            filter.word0 = 1;
            filter.word1 = 1;
            shape->setSimulationFilterData(filter);
        } else {
            EventBus().publish<EditorLogEvent>(EditorLogEvent::Type::eError, "The rigid body has no collider: a mesh collider needs a Mesh with a model\n");
        }

        actor->userData = this;
    }

    void RigidBody::OnUpdate(Transform& transform) noexcept {
        physx::PxTransform t = actor->getGlobalPose();
        if (t.isValid() && t.isSane()) {
            transform.translate = Vector3D{ t.p.x, t.p.y, t.p.z };
            transform.q.x       = t.q.x;
            transform.q.y       = t.q.y;
            transform.q.z       = t.q.z;
            transform.q.w       = t.q.w;

            translate = transform.translate;
            rotation  = transform.rotation;

            if (bodyType == PhysicsBodyType::eDynamic) {
                auto v          = static_cast<physx::PxRigidDynamic*>(actor)->getLinearVelocity();
                velocity        = Vector3D{ v.x, v.y, v.z };
                auto av         = static_cast<physx::PxRigidDynamic*>(actor)->getAngularVelocity();
                angularVelocity = Vector3D{ av.x, av.y, av.z };
            }
        }
    }

    void RigidBody::SetTranslation(float x, float y, float z) noexcept {
        physx::PxTransform t = actor->getGlobalPose();
        t.p                  = physx::PxVec3{ x, y, z };
        actor->setGlobalPose(t);
    }

    Vector3D& RigidBody::GetTranslation() noexcept {
        return translate;
    }

    void RigidBody::SetPosition(float x, float y, float z) noexcept {
        physx::PxTransform t = actor->getGlobalPose();
        physx::PxVec3 p(x, y, z);
        t.p = p;
        actor->setGlobalPose(t);
    }

    void RigidBody::SetRotation(float x, float y, float z) noexcept {
        physx::PxTransform t = actor->getGlobalPose();
        Quaternion qq(Vector3D{ x, y, z });
        physx::PxQuat q(qq.x, qq.y, qq.z, qq.w);
        t.q = q;
        actor->setGlobalPose(t);
    }

    void RigidBody::AddRotation(float x, float y, float z) noexcept {
        physx::PxTransform t = actor->getGlobalPose();
        Quaternion qq(Vector3D{ x, y, z });
        physx::PxQuat q(qq.x, qq.y, qq.z, qq.w);
        t.q *= q;
        actor->setGlobalPose(t);
    }

    Vector3D& RigidBody::GetRotation() noexcept {
        return rotation;
    }

    void RigidBody::SetVelocity(float x, float y, float z) noexcept {
        if (auto* dynamic{ SimulatedActor() }) {
            dynamic->setLinearVelocity(physx::PxVec3(x, y, z));
        }
    }

    void RigidBody::AddVelocity(float x, float y, float z) noexcept {
        if (auto* dynamic{ SimulatedActor() }) {
            auto vel{ dynamic->getLinearVelocity() };
            vel.x += x;
            vel.y += y;
            vel.z += z;
            dynamic->setLinearVelocity(vel);
        }
    }

    Vector3D& RigidBody::GetVelocity() noexcept {
        return velocity;
    }

    void RigidBody::AddForce(float x, float y, float z) noexcept {
        if (auto* dynamic{ SimulatedActor() }) {
            dynamic->addForce(physx::PxVec3(x, y, z));
        }
    }

    void RigidBody::AddTorque(float x, float y, float z) noexcept {
        if (auto* dynamic{ SimulatedActor() }) {
            dynamic->addTorque(physx::PxVec3(x, y, z));
        }
    }

    float RigidBody::GetRestitution() const noexcept {
        return restitution;
    }

    void RigidBody::SetRestitution(float newRestitution) noexcept {
        restitution = newRestitution;
        material->setRestitution(newRestitution);
    }

    float RigidBody::GetStaticFriction() const noexcept {
        return staticFriction;
    }

    void RigidBody::SetStaticFriction(float newStaticFriction) noexcept {
        staticFriction = newStaticFriction;
        material->setStaticFriction(newStaticFriction);
    }

    float RigidBody::GetDynamicFriction() const noexcept {
        return dynamicFriction;
    }

    void RigidBody::SetDynamicFriction(float newDynamicFriction) noexcept {
        dynamicFriction = newDynamicFriction;
        material->setDynamicFriction(newDynamicFriction);
    }

    float RigidBody::GetMass() const noexcept {
        return mass;
    }

    void RigidBody::SetMass(float newMass) noexcept {
        mass = newMass;
        if (bodyType == PhysicsBodyType::eDynamic) {
            physx::PxRigidBodyExt::updateMassAndInertia(*static_cast<physx::PxRigidDynamic*>(actor), newMass);
        }
    }

    void RigidBody::SetAngularVelocity(float x, float y, float z) noexcept {
        if (auto* dynamic{ SimulatedActor() }) {
            dynamic->setAngularVelocity(physx::PxVec3(x, y, z));
        }
    }

    void RigidBody::AddAngularVelocity(float x, float y, float z) noexcept {
        if (auto* dynamic{ SimulatedActor() }) {
            auto vel{ dynamic->getAngularVelocity() };
            vel.x += x;
            vel.y += y;
            vel.z += z;
            dynamic->setAngularVelocity(vel);
        }
    }

    Vector3D& RigidBody::GetAngularVelocity() noexcept {
        return angularVelocity;
    }

    void RigidBody::SetHasGravity(bool hasGravity) noexcept {
        actor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, !hasGravity);
    }

    void RigidBody::SetTrigger(bool newIsTrigger) noexcept {
        isTrigger    = newIsTrigger;
        colliderType = newIsTrigger ? PhysicsColliderType::eTrigger : PhysicsColliderType::eCollider;
        if (!shape) {
            return;
        }
        if (newIsTrigger) {
            shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
            shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
        } else {
            shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
            shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
        }
    }

    void RigidBody::SetLinearFactor(bool x, bool y, bool z) noexcept {
        if (bodyType == PhysicsBodyType::eDynamic) {
            static_cast<physx::PxRigidDynamic*>(actor)->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X, x);
            static_cast<physx::PxRigidDynamic*>(actor)->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y, y);
            static_cast<physx::PxRigidDynamic*>(actor)->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, z);
        }
    }

    void RigidBody::SetAngularFactor(bool x, bool y, bool z) noexcept {
        if (bodyType == PhysicsBodyType::eDynamic) {
            static_cast<physx::PxRigidDynamic*>(actor)->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, x);
            static_cast<physx::PxRigidDynamic*>(actor)->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, y);
            static_cast<physx::PxRigidDynamic*>(actor)->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, z);
        }
    }

    bool RigidBody::GetIsTrigger() const noexcept {
        return isTrigger;
    }

    void RigidBody::SetKinematic(bool newIsKinematic) noexcept {
        if (bodyType == PhysicsBodyType::eDynamic) {
            isKinematic = newIsKinematic;
            static_cast<physx::PxRigidDynamic*>(actor)->setRigidBodyFlag(physx::PxRigidBodyFlag::Enum::eKINEMATIC, newIsKinematic);
        }
    }

    void RigidBody::SetGeometry(const physx::PxGeometry& geometry) {
        if (shape) {
            shape->setGeometry(geometry);
        }
    }

    void RigidBody::UpdateGeometry() noexcept {
        if (!shape) {
            return;
        }
        if (colliderShape == PhysicsColliderShape::eBox) {
            SetGeometry(physx::PxBoxGeometry{ scale.x, scale.y, scale.z });
        } else if (colliderShape == PhysicsColliderShape::eSphere) {
            SetGeometry(physx::PxSphereGeometry{ radius });
        } else if (colliderShape == PhysicsColliderShape::eCapsule) {
            SetGeometry(physx::PxCapsuleGeometry{ radius, halfHeight });

        } else if (colliderShape == PhysicsColliderShape::eMesh) {
            physx::PxTriangleMeshGeometry geometry2;
            shape->getTriangleMeshGeometry(geometry2);
            physx::PxMeshScale s(physx::PxVec3(scale[0], scale[1], scale[2]), physx::PxQuat(physx::PxIdentity));
            geometry2.triangleMesh->acquireReference();
            shape->setGeometry(physx::PxTriangleMeshGeometry(geometry2.triangleMesh, s));
            geometry2.triangleMesh->release();
            // SetGeometry(physx::PxTriangleMeshGeometry(geometry2.triangleMesh, s));

        } else if (colliderShape == PhysicsColliderShape::eConvexMesh) {
            physx::PxConvexMeshGeometry geometry2;
            shape->getConvexMeshGeometry(geometry2);
            physx::PxMeshScale s(physx::PxVec3(scale[0], scale[1], scale[2]), physx::PxQuat(physx::PxIdentity));
            geometry2.convexMesh->acquireReference();
            shape->setGeometry(physx::PxConvexMeshGeometry(geometry2.convexMesh, s));
            geometry2.convexMesh->release();
            // SetGeometry(physx::PxConvexMeshGeometry(geometry2.convexMesh, s));
        }
    }

    void RigidBody::ClearForces() noexcept {
        if (auto* dynamic{ SimulatedActor() }) {
            dynamic->clearForce(physx::PxForceMode::eFORCE);
            dynamic->clearForce(physx::PxForceMode::eIMPULSE);
            dynamic->clearForce(physx::PxForceMode::eACCELERATION);
            dynamic->clearForce(physx::PxForceMode::eVELOCITY_CHANGE);
            dynamic->clearTorque();
            dynamic->setLinearVelocity(physx::PxVec3{ 0.0f, 0.0f, 0.0f }, false);
            dynamic->setAngularVelocity(physx::PxVec3{ 0.0f, 0.0f, 0.0f }, false);
            velocity        = {};
            angularVelocity = {};
        }
    }

    physx::PxRigidDynamic* RigidBody::SimulatedActor() const noexcept {
        if (bodyType != PhysicsBodyType::eDynamic) {
            return nullptr;
        }
        auto* dynamic{ static_cast<physx::PxRigidDynamic*>(actor) };
        return dynamic->getRigidBodyFlags() & physx::PxRigidBodyFlag::eKINEMATIC ? nullptr : dynamic;
    }

    void RigidBody::Destroy() noexcept {
        Clear();
    }

    void RigidBody::MoveConstruct(RigidBody&& rhs) noexcept {
        staticFriction   = rhs.staticFriction;
        dynamicFriction  = rhs.dynamicFriction;
        restitution      = rhs.restitution;
        material         = rhs.material;
        isKinematic      = rhs.isKinematic;
        mass             = rhs.mass;
        bodyType         = rhs.bodyType;
        actor            = rhs.actor;
        colliderShape    = rhs.colliderShape;
        colliderType     = rhs.colliderType;
        shape            = rhs.shape;
        scaleSameAsModel = rhs.scaleSameAsModel;
        scale            = rhs.scale;
        radius           = rhs.radius;
        halfHeight       = rhs.halfHeight;
        entity           = rhs.entity;
        isTrigger        = rhs.isTrigger;
        velocity         = rhs.velocity;
        angularVelocity  = rhs.angularVelocity;
        translate        = rhs.translate;
        rotation         = rhs.rotation;

        if (actor != nullptr) {
            actor->userData = this;
        }
        rhs.actor    = nullptr;
        rhs.material = nullptr;
        rhs.shape    = nullptr;
    }

    void RigidBody::Clear() noexcept {
        if (material) {
            material->release();
            material = nullptr;
        }
        if (actor) {
            actor->getScene()->removeActor(*actor);
            actor->release();
            shape = nullptr;
            actor = nullptr;
        }
    }
} // namespace adh
