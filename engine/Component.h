#pragma once

namespace ECSEngine {

/// Dense storage for every component of type T owned by one EntityManager.
///
/// This is an internal structure. EntityManager::GetComponentStorage() exposes
/// it for systems that want to scan all components of one type, but in most
/// cases you should use EntityManager::GetComponent<T>(id) instead.
///
/// Layout and terms of use:
///  - Indices into these vectors are COMPONENT SLOT indices, not EntityIDs.
///    There is no public mapping from a slot back to its entity.
///  - mStorage[i] is live only when mValid[i] is true. A removed component
///    stays in mStorage holding stale data until its slot is reused, so a
///    scan must skip slots where mValid[i] is false.
///  - Adding a component of type T can reallocate mStorage, which invalidates
///    every pointer, reference and iterator into it. Do not add components
///    of type T while you hold any of them.
///  - Do not modify these vectors directly. Use AddComponent/RemoveComponent.
template <typename T>
class ComponentStorage {
public:
    std::vector<T> mStorage;        // Component values, indexed by slot.
    std::vector<bool> mValid;       // mValid[i]: slot i holds a live component.
    std::vector<size_t> mFreeList;  // Dead slots available for reuse (LIFO).
};

}  // namespace ECSEngine
