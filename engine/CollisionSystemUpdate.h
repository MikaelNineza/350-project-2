#pragma once

#include <string>

#include "CollisionComponent.h"
#include "LocationComponent.h"
#include "MathUtil.h"
#include "ECSEngine.h"

namespace ECSEngine {
    template <typename... Components>
    class CollisionSystemUpdate : public System<Components...> {
    public:
        tSystemReturn Run(EngineView<Components...>* engine) override {
            auto &em = engine->GetEntityManager();
            for (const auto &entity : em) {
                EntityID id = entity.GetID();
                if (!em.template HasComponent<CollisionComponent>(id)) {
                    continue;
                }

                auto &col = em.template GetComponent<CollisionComponent>(id);
                if (col.staticEntity) {
                    continue;
                }


                auto &loc = em.template GetComponent<LocationComponent>(id);
                col.prevCollisionBox = col.collisionBox;
                col.prevCollisionBox.topLeft += loc.loc; // location is top left
            }

            return kContinue;
        }

        const std::string &GetName() const override {
            return mName;
        }
    private:
        std::string mName = "CollisionSystemUpdate";
    };
}
