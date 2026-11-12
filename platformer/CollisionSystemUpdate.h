#include <vector>

#include "CollisionComponent.h"
#include "LocationComponent.h"
#include "MathUtil.h"
#include "ECSEngine.h"

template <typename... Components>
class CollisionSystemUpdate : public ECSEngine::System<Components...> {
public:
    ECSEngine::tSystemReturn Run(ECSEngine::EngineView<Components...>* engine) override {
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
            col.prevCollisionBox.topLeft += loc.loc; // location is top left, not sure if this is right
        }
    }

    const std::string &GetName() const override {
        return mName;
    }
private:
    using EntityID = ECSEngine::EntityID;
    std::string mName = "CollisionSystemUpdate";
};
