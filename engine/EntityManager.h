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

template <typename... Components>
class EntityManager {
public:
    static constexpr size_t kNullComponent = std::numeric_limits<size_t>::max();

    template <typename Component>
    static constexpr EntityID ComponentToIndex() {
        constexpr int temp = Pack<Components...>::template index<Component>;
        static_assert(temp != -1, "Component does not exist.");
        return static_cast<EntityID>(temp);
    }

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

    void RemoveEntity(EntityID entity) {
        assert(ValidEntity(entity));
        // https://stackoverflow.com/a/25683817
        ((void) (HasComponent<Components>(entity) && (RemoveComponent<Components>(entity), 0)), ...);
        mFreeList.push_back(entity);
        mValid[entity] = false;
    }

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

    template <typename T>
    bool HasComponent(EntityID entity) const {
        constexpr EntityID typeIdx = ComponentToIndex<T>();
        assert(ValidEntity(entity));
        return mComponentIndices[typeIdx][entity] != kNullComponent;
    }

    template <typename T>
    T& GetComponent(EntityID entity) {
        constexpr EntityID typeIdx = ComponentToIndex<T>();
        assert(HasComponent<T>(entity));
        auto& componentStorage = GetComponentStorage<T>();
        return componentStorage.mStorage[mComponentIndices[typeIdx][entity]];
    }

    template <typename T>
    ComponentStorage<T>& GetComponentStorage() {
        return std::get<ComponentStorage<T>>(mComponentStorages);
    }

    bool ValidEntity(EntityID id) const {
        return mValid.size() > id && mValid[id];
    }

    using tEntity = std::vector<Entity>;
	using const_iterator = tEntity::const_iterator;
	const_iterator cbegin() const { return mEntities.begin(); }
	const_iterator cend() const { return mEntities.end(); }
	const_iterator begin() { return mEntities.begin(); }
	const_iterator end() { return mEntities.end(); }

private:
    std::string name = "EntityManager";
    std::tuple<ComponentStorage<Components>...> mComponentStorages;
    std::array<std::vector<size_t>, sizeof...(Components)> mComponentIndices;
    std::vector<bool> mValid;
    std::vector<EntityID> mFreeList;
    std::vector<Entity> mEntities;
};

}  // namespace ECSEngine
