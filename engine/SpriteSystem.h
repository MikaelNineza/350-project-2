#pragma once

#include <bitset>
#include <unordered_map>

#include "CameraComponent.h"
#include "EntityManager.h"
#include "LocationComponent.h"
#include "MathUtil.h"
#include "SpriteManager.h"
#include "SystemManager.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace ECSEngine {

template <typename... Components>
class SpriteSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("SpriteSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto &em = engine->GetEntityManager();
        auto &sm = engine->GetSpriteManager();
        auto window = engine->GetWindowManager().GetWindow();
        window->clear(sf::Color::Black);

        for (const auto &entity : em) {
            EntityID id = entity.GetID();
            if (!em.template HasComponent<SpriteComponent>(id)) {
                continue;
            }
            if (!em.template HasComponent<LocationComponent>(id)) {
                continue;
            }

            SpriteComponent sComponent = em.template GetComponent<SpriteComponent>(id);
            Point2D location = em.template GetComponent<LocationComponent>(id).loc;
            sf::Sprite &sprite = sm.GetSprite(sComponent.textureID);
            sf::IntRect textureRect = sprite.getTextureRect();
            Rect bounds = sComponent.bounds;
            bounds.topLeft = bounds.topLeft + location; // makes bounds world coords
            if (sComponent.worldSpace) {
                bounds = engine->GetWindowManager().WorldToWindow(bounds);
            }

            sprite.setPosition({bounds.topLeft.x, bounds.topLeft.y});
            // https://stackoverflow.com/a/21541729, added {} since we use 3.1
            sprite.setScale({
                bounds.width / textureRect.size.x,
                bounds.height / textureRect.size.y});
            window->draw(sprite);
        }

        window->display();
        return kContinue;
    }
};

}  // namespace ECSEngine
