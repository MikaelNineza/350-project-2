#pragma once

#include <string>

#include "Entity.h"
#include "LocationComponent.h"
#include "ECSEngine.h"

namespace ECSEngine {

template <typename... Components>
class MovementSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("MovementSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto &em = engine->GetEntityManager();
        for (auto &entity : em) {
            EntityID id = entity.GetID();
            if (!em.template HasComponent<MovementComponent>(id) ||
                !em.template HasComponent<LocationComponent>(id)) {
                continue;
            }

            auto &mov = em.template GetComponent<MovementComponent>(id);
            auto &loc = em.template GetComponent<LocationComponent>(id);

            loc.loc = loc.loc + mov.velocity / 60.f;
        }
        return kContinue;
    }
};

}  // namespace ECSEngine
