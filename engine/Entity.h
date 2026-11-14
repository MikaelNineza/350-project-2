#pragma once

#include <array>
#include <tuple>

namespace ECSEngine {

/// Handle that identifies an entity inside one EntityManager.
///
/// Terms of use:
///  - An EntityID is only meaningful for the EntityManager that issued it.
///  - IDs are RECYCLED. After EntityManager::RemoveEntity(id), a later
///    CreateEntity() may return the same numeric id for a brand-new entity.
///    There is no generation counter, so a stale id can silently alias the new
///    entity. If you keep an id across frames (e.g. CameraFollower::entityID),
///    you must make sure the entity it names is never removed, or drop your
///    copy when it is.
///  - Use EntityManager::ValidEntity(id) to check whether an id currently
///    names a live entity.
using EntityID = size_t;

/// Bookkeeping record for one entity slot.
///
/// An Entity holds no components itself. Components are stored by the
/// owning EntityManager and looked up by id. Entities can only be made by
/// EntityManager (the constructors are private), and the manager owns them.
/// Client code sees them only through the manager's iterators, and should
/// use them only to call GetID().
///
/// Lifetime: an `const Entity&` taken from iteration is valid only until the
/// next EntityManager::CreateEntity() call (see EntityManager iteration).
class Entity {
public:
    /// The id of the slot this record lives in. The slot may be dead (removed).
    /// Always check EntityManager::ValidEntity(GetID()) before using it.
    EntityID GetID() const { return mID; }

private:
    Entity() = default;
    Entity(EntityID eid, const std::string& name) : mID(eid), mType(name) {}
    EntityID mID;
    std::string mType;  // Free-form type/debug name given to CreateEntity().

    template <typename... T>
    friend class EntityManager;
};

}  // namespace ECSEngine
