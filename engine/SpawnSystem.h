#pragma once

#include <random>
#include <string>
#include <vector>

#include "Entity.h"
#include "MathUtil.h"
#include "SystemManager.h"
#include "LocationComponent.h"
#include "SpawnComponent.h"
#include "SpriteComponent.h"
#include "MovementComponent.h"
#include "GravityComponent.h"
#include "CollisionComponent.h"

namespace ECSEngine {

template <typename... Components>
class SpawnSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("SpawnSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto &em = engine->GetEntityManager();
        std::vector<SpawnItem> spawnItems;
        for (const auto &entity : em) {
            EntityID id = entity.GetID();
            if (!em.ValidEntity(id)) {
					continue;
			}
            if (!em.template HasComponent<SpawnComponent>(id)) {
                continue;
            }

            SpawnComponent &spawnComp = em.template GetComponent<SpawnComponent>(id);
            if (spawnComp.totalSpawns == 0) {
                continue;
            }

            if (!em.template HasComponent<LocationComponent>(id) ||
                !em.template HasComponent<SpriteComponent>(id)) {
                continue;
            }

            spawnComp.timeToNextSpawn -= 1;
            if (spawnComp.timeToNextSpawn > 0) {
                continue;
            }

            spawnComp.totalSpawns -= 1;
            spawnComp.timeToNextSpawn = spawnComp.timeBetweenSpawns;

            SpawnItem item;
            item.spawnType = spawnComp.spawnType;
            item.textureID = spawnComp.spawnTextureID;
            item.spawnLoc = em.template GetComponent<LocationComponent>(id).loc;
            item.bounds = em.template GetComponent<SpriteComponent>(id).bounds;
            spawnItems.push_back(item);
        }

        for (const SpawnItem &item : spawnItems) {
            EntityID star = em.CreateEntity(item.spawnType);
            float dir = (mRandom() % 2) ? 1.f : -1.f;
            
            em.AddComponent(star, LocationComponent{item.spawnLoc});
            em.AddComponent(star, MovementComponent{Point2D(dir * kStarVeloX, 0.f)});
            em.AddComponent(star, GravityComponent(Point2D(0.f, kStarGravity)));

            CollisionComponent col{};
            col.staticEntity = 0;
            // The values for this box were chosen from Prompt #2 (see readme)
            col.collisionBox = Rect{Point2D(16.f, 16.f), 35, 35};

            col.previousCollisionBox = item.bounds;
            col.previousCollisionBox.topLeft = item.bounds.topLeft + item.spawnLoc;
            em.AddComponent(star, col);

            SpriteComponent sprite;
            sprite.textureID = item.textureID;
            sprite.bounds = item.bounds;
            sprite.worldSpace = true;
            em.AddComponent(star, sprite);
        }

        return kContinue;
    }
private:
    static constexpr float kStarVeloX = 100.f;
    static constexpr float kStarGravity = 1000.f;
    std::mt19937 mRandom;

    struct SpawnItem {
        Point2D spawnLoc;
        std::string spawnType;
        SpriteID textureID;
        Rect bounds;
    };
};

}  // namespace ECSEngine
