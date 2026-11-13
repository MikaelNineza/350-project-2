#pragma once

#include "ScoreComponent.h"
#include "SpriteComponent.h"
#include "SystemManager.h"
namespace ECSEngine {

template <typename... Components>
class ScoreSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("ScoreSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto &em = engine->GetEntityManager();

        for (const auto &entity : em) {
            EntityID id = entity.GetID();
            if (!em.template HasComponent<ScoreComponent>(id)) {
                continue;
            }

            ScoreComponent &sComponent = em.template GetComponent<ScoreComponent>(id);
            if (sComponent.score > 999) {
                sComponent.score = 999;
            }
            int digits[3] = {sComponent.score / 100, (sComponent.score / 10) % 10, sComponent.score % 10};
            for (int i = 0; i < 3; ++i) {
                EntityID digitID = sComponent.digitID[i];
                if (!em.template HasComponent<SpriteComponent>(digitID)) {
                    continue;
                }

                SpriteComponent &spriteComponent = em.template GetComponent<SpriteComponent>(digitID);
                spriteComponent.textureID = sComponent.digitTextureID[digits[i]];
            }

        }
        return kContinue;
    }
};

}  // namespace ECSEngine
