#pragma once

#include <cassert>
#include <iostream>
#include <limits>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include "Component.h"
#include "Entity.h"
#include "Pack.h"

namespace ECSEngine {

/// Owns every entity and every component in the game.
///
/// `Components...` is the closed list of component types this manager can
/// store. Using a type that is not in the list is a compile-time error.
///
/// Ownership: the EntityManager owns all Entity records and all component
/// values. Client code holds only EntityIDs (non-owning handles, see Entity.h)
/// and short-lived references returned by GetComponent().
///
/// In an ECSEngine, the engine owns the single EntityManager and it lives as
/// long as the engine does. Get it from EngineView::GetEntityManager().
///
/// REFERENCE VALIDITY RULES (summary, details on each method):
///
///  | You hold                          | It is invalidated by                     |
///  |-----------------------------------|------------------------------------------|
///  | T& from GetComponent<T>(e)        | AddComponent<T>(any entity)              |
///  |                                   | RemoveComponent<T>(e) / RemoveEntity(e)  |
///  | iterator / Entity& from iteration | CreateEntity()                           |
///  | ComponentStorage<T>&              | never (lives as long as the manager)     |
///  | EntityID                          | RemoveEntity(id) (and the id may later   |
///  |                                   | be reused by CreateEntity)               |
///
/// In practice: get component references fresh inside each system Run() and
/// never store them across frames. If a system needs to create entities while
/// iterating, record what to create and do it after the loop (SpawnSystem
/// does this).
///
/// Thread safety: none. The engine is single-threaded.
template <typename... Components>
class EntityManager {
public:
    /// Sentinel component-slot index meaning "this entity has no such component".
    static constexpr size_t kNullComponent = std::numeric_limits<size_t>::max();

    /// Compile-time position of `Component` in the `Components...` list.
    /// Fails to compile (static_assert) if the type is not registered.
    template <typename Component>
    static constexpr EntityID ComponentToIndex() {
        constexpr int temp = Pack<Components...>::template index<Component>;
        static_assert(temp != -1, "Component does not exist.");
        return static_cast<EntityID>(temp);
    }

    /// Creates a new entity with no components and returns its id.
    ///
    /// @param name  Free-form type name (e.g. "player", "star"). It is copied,
    ///              and is for debugging/labelling only.
    /// @return      An id that is valid until RemoveEntity(id). It may be a
    ///              recycled id of an earlier, removed entity.
    ///
    /// Invalidates: all iterators and Entity references from begin()/end(),
    /// because the entity table may grow. Never call this inside a
    /// range-for over the manager. Existing component references are NOT
    /// invalidated.
    [[nodiscard]] EntityID CreateEntity(const std::string& name) {
        EntityID id;
        if (!mFreeList.empty()) {
            id = mFreeList.back();
            mFreeList.pop_back();
            mEntities[id] = Entity(id, name);
            mValid[id] = true;
        } else {
            id = mValid.size();
            for (auto& indices : mComponentIndices) {
                indices.push_back(kNullComponent);
            }
            mEntities.push_back(Entity(id, name));
            mValid.push_back(true);
        }

        return id;
    }

    /// Destroys an entity and all of its components.
    ///
    /// Precondition: ValidEntity(entity) (asserted).
    ///
    /// After this call the id is dead and goes on a free list. The next
    /// CreateEntity() may hand out the same id again, so drop every stored
    /// copy of it. References to this entity's components point at dead
    /// slots that a later AddComponent may reuse, so they must not be used.
    ///
    /// It is safe to call this while iterating the manager: the entity table
    /// does not shrink, and the slot is only marked dead, so iteration still
    /// visits it. That is why loops must check ValidEntity().
    void RemoveEntity(EntityID entity) {
        assert(ValidEntity(entity));
        // https://stackoverflow.com/a/25683817
        ((void) (HasComponent<Components>(entity) && (RemoveComponent<Components>(entity), 0)), ...);
        mFreeList.push_back(entity);
        mValid[entity] = false;
    }

    /// Attaches a component to an entity. The manager takes ownership of it.
    ///
    /// @param entity     Must be valid and must not already have a T (asserted).
    /// @param component  Taken by value and moved into storage. The caller's
    ///                   object is not linked to the stored copy, so read it
    ///                   back with GetComponent<T>() to modify it later.
    ///
    /// Invalidates: every T& previously returned by GetComponent<T>() for
    /// ANY entity, and every pointer/iterator into GetComponentStorage<T>(),
    /// because the storage vector may reallocate. References to other
    /// component types are unaffected.
    template <typename T>
    void AddComponent(EntityID entity, T component) {
        constexpr EntityID typeIdx = ComponentToIndex<T>();
        assert(!HasComponent<T>(entity));
        auto& componentStorage = GetComponentStorage<T>();
        size_t id;
        if (!componentStorage.mFreeList.empty()) {
            id = componentStorage.mFreeList.back();
            componentStorage.mFreeList.pop_back();
            componentStorage.mStorage[id] = std::move(component);
            componentStorage.mValid[id] = true;
        } else {
            id = componentStorage.mStorage.size();
            componentStorage.mStorage.push_back(std::move(component));
            componentStorage.mValid.push_back(true);
        }

        mComponentIndices[typeIdx][entity] = id;
    }

    /// Detaches and discards the entity's T component.
    ///
    /// Precondition: HasComponent<T>(entity) (asserted).
    ///
    /// The value is not destroyed right away. Its slot is marked dead and kept
    /// for reuse, so the slot keeps its old data until the next
    /// AddComponent<T> overwrites it. Any T& you held for this entity is now
    /// dangling in the logical sense. References to other entities' T
    /// components stay valid.
    template <typename T>
    void RemoveComponent(EntityID entity) {
        constexpr EntityID typeIdx = ComponentToIndex<T>();
        assert(HasComponent<T>(entity));
        auto& componentStorage = GetComponentStorage<T>();
        size_t id = mComponentIndices[typeIdx][entity];
        componentStorage.mFreeList.push_back(id);
        componentStorage.mValid[id] = false;
        mComponentIndices[typeIdx][entity] = kNullComponent;
    }

    /// True if the entity currently has a component of type T.
    /// Precondition: ValidEntity(entity) (asserted). Check validity first
    /// when walking the entity list.
    template <typename T>
    bool HasComponent(EntityID entity) const {
        constexpr EntityID typeIdx = ComponentToIndex<T>();
        assert(ValidEntity(entity));
        return mComponentIndices[typeIdx][entity] != kNullComponent;
    }

    /// Returns a mutable reference to the entity's T component.
    ///
    /// Precondition: HasComponent<T>(entity) (asserted).
    ///
    /// The reference points into the manager's storage. It is NOT owned by
    /// the caller and is only guaranteed valid until the next of:
    ///   - AddComponent<T>(...) on any entity (storage may reallocate),
    ///   - RemoveComponent<T>(entity) or RemoveEntity(entity),
    ///   - destruction of the EntityManager (i.e. the engine).
    /// Treat it as scoped to the current statement or block. Copy the value
    /// if you need it later, and do not store the reference in a system
    /// between frames.
    template <typename T>
    T& GetComponent(EntityID entity) {
        constexpr EntityID typeIdx = ComponentToIndex<T>();
        assert(HasComponent<T>(entity));
        auto& componentStorage = GetComponentStorage<T>();
        return componentStorage.mStorage[mComponentIndices[typeIdx][entity]];
    }

    /// Low-level access to every component of type T (see ComponentStorage).
    ///
    /// The returned reference to the storage object is valid for the whole
    /// lifetime of the manager. However, references and iterators INTO its
    /// vectors follow the same rules as GetComponent(). Slots are not
    /// EntityIDs, and dead slots must be skipped using mValid. Do not modify
    /// the vectors directly.
    template <typename T>
    ComponentStorage<T>& GetComponentStorage() {
        return std::get<ComponentStorage<T>>(mComponentStorages);
    }

    /// True if `id` was issued by this manager and has not been removed.
    /// Safe to call with any value, including ids that were never issued.
    bool ValidEntity(EntityID id) const {
        return mValid.size() > id && mValid[id];
    }

    /// Iteration over every entity slot ever created, INCLUDING removed ones.
    ///
    /// Usage pattern used by all systems:
    ///   for (const auto& entity : em) {
    ///       EntityID id = entity.GetID();
    ///       if (!em.ValidEntity(id)) continue;   // required: skip dead slots
    ///       ...
    ///   }
    ///
    /// Iterators are read-only (const_iterator). They and the Entity
    /// references they produce are invalidated by CreateEntity(). They are
    /// not invalidated by RemoveEntity(), AddComponent() or RemoveComponent().
    using tEntity = std::vector<Entity>;
	using const_iterator = tEntity::const_iterator;
	const_iterator cbegin() const { return mEntities.begin(); }
	const_iterator cend() const { return mEntities.end(); }
	const_iterator begin() { return mEntities.begin(); }
	const_iterator end() { return mEntities.end(); }

private:
    std::string name = "EntityManager";
    // One ComponentStorage per component type.
    std::tuple<ComponentStorage<Components>...> mComponentStorages;
    // mComponentIndices[type][entity] = slot in that type's storage, or kNullComponent.
    std::array<std::vector<size_t>, sizeof...(Components)> mComponentIndices;
    std::vector<bool> mValid;         // mValid[entity]: entity is alive.
    std::vector<EntityID> mFreeList;  // Removed ids available for reuse (LIFO).
    std::vector<Entity> mEntities;    // One record per slot, alive or dead.
};

}  // namespace ECSEngine
