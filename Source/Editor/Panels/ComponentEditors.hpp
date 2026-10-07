#pragma once
#include <adh/entity.hpp>

#include <span>

namespace adh {
    struct EditorContext;

    enum class AddKind {
        eDefault,
        eRigidBody,
        eScript
    };

    struct ComponentEditor {
        const char* name;
        const char* icon{};
        bool (*has)(ecs::World& world, ecs::Entity entity){};
        void (*draw)(EditorContext& context, ecs::Entity entity){};
        void (*reset)(ecs::World& world, ecs::Entity entity){};
        void (*remove)(ecs::World& world, ecs::Entity entity){};
        AddKind addKind{ AddKind::eDefault };
        void (*add)(EditorContext& context, ecs::Entity entity){};
        const char* (*whyNotAddable)(EditorContext& context, ecs::Entity entity){};
    };

    std::span<const ComponentEditor> GetComponentEditors();
} // namespace adh
