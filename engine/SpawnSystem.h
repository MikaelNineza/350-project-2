#pragma once

#include "LocationComponent.h"
#include "SpawnComponent.h"

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
        std::vector<SpriteID> newEntities; //TODO: Figure out what should be the type of this
        for (const auto &entity : em) {
            EntityID id = entity.GetID();
            if (!em.template HasComponent<SpawnComponent>(id)) {
                continue;
            }

            SpawnComponent &spawnComp = em.template GetComponent<SpawnComponent>(id);
            if (spawnComp.totalSpawns == 0) {
                continue;
            }

            EntityID entity = spawnComp.entityID;
            if (!em.template HasComponent<LocationComponent>(entity)) {
                continue;
            }

            spawnComp.timeToNextSpawn -= 1;
            if (spawnComp.timeToNextSpawn) {
                continue;
            }

            //TODO: add info for what will be spawn to the vector

            spawnComp.totalSpawns -= 1;
            spawnComp.timeToNextSpawn = spawnComp.timeBetweenSpawns;
        }

        //TODO: iterate over vector and create entities


        return kContinue;
    }
};

}  // namespace ECSEngine
